/**
 * @file
 */

#define BOOST_CHRONO_VERSION 2
#include <appcon/config.h>

#include <memory>
#include <mutex>
#include <map>
#include <unordered_map>
#include <tuple>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

#include <boost/any.hpp>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <vector>
#include <boost/variant.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/config.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#define TRACE BOOST_LOG_TRIVIAL(trace)
#define DEBUG BOOST_LOG_TRIVIAL(debug)
#define INFO BOOST_LOG_TRIVIAL(info)
#define WARN BOOST_LOG_TRIVIAL(warn)
#define ERROR BOOST_LOG_TRIVIAL(error)
#define FATAL BOOST_LOG_TRIVIAL(fatal)

namespace appcon {
namespace detail {

/** std::to_string() with support for transparent string passthrough */
template<typename T> inline
std::string to_string(const T v) { return std::to_string(v); }
template<> inline
std::string to_string(const std::string &v) { return v; }
template<> inline
std::string to_string(const std::string v) { return v; }

namespace {

/** Used for hashing the type info for map keys */
struct type_info_hash {
	std::size_t operator()(std::type_info const & t) const {
		return t.hash_code();
	}
};

/** Used for testing types for equality */
struct equal_ref {
	template <typename T>
	bool operator()(
		std::reference_wrapper<T> a,
		std::reference_wrapper<T> b
	) const {
		return a.get() == b.get();
	}
};

/** More of a type registry than a visitor */
class any_visitor {
public:
	using type_info_ref = std::reference_wrapper<std::type_info const>;
	using function = std::function<std::string(const std::string &, boost::any&, const std::string &)>;
	/** We store type info directly for our function lookups, so we also provide hashing and equality */
	std::unordered_map<
		type_info_ref,
		function,
		type_info_hash,
		equal_ref
	> fs;

	/**
	 * This is used to populate the type => function lookup table.
	 */
	template <typename T>
	void insert_visitor(std::function<std::string(const std::string &, T&, const std::string &)> f) {
		fs.insert(
			std::make_pair(
				std::ref(typeid(T)),
				[f](const std::string &k, boost::any &x, const std::string &src) -> std::string {
					auto v = boost::any_cast<T&>(x);
					// std::cout << "Calling f() for " << k << " src " << src << "\n";// << type_id_with_cvr<decltype(casted)>().pretty_name() << "\n";
					return f(k, v, src);
				}
			)
		);
	}

	/**
	 * Dispatch a type to our lookup table, calling the matching function if found.
	 * @throws std::runtime_error if none found
	 */
	std::string operator()(const std::string &k, boost::any &x, const std::string &src) {
		auto it = fs.find(x.type());
		if (it != fs.end()) {
			return it->second(k, x, src);
		}
		throw std::runtime_error("No type handler registered");
	}
};

/**
 * Provides the templated operator for dispatching key/value/source information
 * to the appropriate appcon::config ->set overload.
 */
class handler {
public:
	handler(
		any_visitor &a,
		appcon::config *c
	):av_(a),cfg_(c) {
	}

	template<typename T>
	void operator()(T&) {
		auto cfg = cfg_;
		av_.insert_visitor<T>([cfg](const std::string &k, T &v, const std::string &src) {
			auto s = to_string(v);
			cfg->set(k, v, src);
			return s;
		});
	}

protected:
	any_visitor &av_;
	appcon::config *cfg_;
};

};

class config : virtual public appcon::config {
public:
	using types = boost::mpl::list<
		uint8_t, uint16_t, uint32_t, uint64_t,
		int8_t, int16_t, int32_t, int64_t,
		float, std::string
	>;
	using storage_type = boost::make_variant_over<types>::type;
	using current_type = std::tuple<
		storage_type, // value
		std::string, // source
		boost::chrono::high_resolution_clock::time_point // last changed
	>;

	config(
	):strict_mode_{ false },
	  visitor_{ },
	  handler_{ visitor_, this },
	  options_desc_("Supported options")
	{
		/* Apply our handlers for known types */
		boost::mpl::for_each<types>(handler_);
	}

	config(const config &src) = default;
	config(config &&src) = default;
	virtual ~config() = default;

	/** Record a new config entry */
	virtual config &operator()( std::string k, const std::string &def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, float def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, uint8_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, uint16_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, uint32_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, uint64_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, int8_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, int16_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, int32_t def, std::string desc) override { add_option(k, def, desc); return *this; }
	virtual config &operator()( std::string k, int64_t def, std::string desc) override { add_option(k, def, desc); return *this; }

	/** Indicates that we should also pull data from the environment, with the given prefix */
	virtual config &from_environment(const std::string &prefix) override {
		loaders_.push_back([this, prefix]() {
			apply_environment(prefix);
		});
		reload();
		return *this;
	}

	/** Provides commandline data - this will be stored in the config object */
	virtual config &from_args(int argc, const char *argv[]) override {
		auto copied = std::vector<std::string> { };
		for(uint_fast16_t i = 0; i < static_cast<uint_fast16_t>(argc); ++i) {
			copied.push_back(std::string { argv[i] });
		}
		loaders_.push_back([this, copied]() {
			int argc = static_cast<int>(copied.size());
			const char *argv[argc];
			for(uint_fast16_t i = 0; i < static_cast<uint_fast16_t>(argc); ++i) {
				argv[i] = static_cast<const char *>(copied[i].data());
			}
			apply_args(argc, argv);
		});
		reload();
		return *this;
	}

	/** config file */
	virtual config &from_file(const std::string &path) override {
		loaders_.push_back([this, path]() {
			apply_file(path);
		});
		reload();
		return *this;
	}

	/**
	 * When set, will throw an exception if we try to look up a value that's either not
	 * been defined at all, or has a different default value from the one we have configured.
	 */
	virtual config &strict(bool v) override {
		strict_mode_ = v;
		return *this;
	}
	/** Set a local override */
	virtual bool have_key(std::string k) const override { return current_.count(k) > 0; }
	virtual const std::string &description(const std::string &k) const override { return description_.at(k); }
	/** Watch a config var */
	virtual std::shared_ptr<watcher> watch(const std::string &k, std::function<void(std::string, std::string)> code) const override { return watch_as<std::string>(k, code); }
	virtual std::shared_ptr<watcher> watch(const std::string &k, std::function<void(float, float)> code) const override { return watch_as<float>(k, code); }

	/** Stop watching */
	virtual config &unwatch(std::shared_ptr<watcher> w) override { return *this; }
	/** Uses local overrides, commandline, environment, config file in that order */
	virtual config &apply() override { return *this; }
	/** Alias for apply() */
	virtual config &reload() override {
		for(auto &code : loaders_) {
			code();
		}
		return *this;
	}
	virtual const config &each_as_string(std::function<void(std::string, std::string)> code) const override {
		for(const auto &it : current_as_string_) {
			code(it.first, it.second);
		}
		return *this;
	}

	virtual std::string key(const std::string &k, const std::string default_value) const override { return key<std::string>(k, default_value); }
	virtual float key(const std::string &k, const float default_value) const override { return key<float>(k, default_value); }
	virtual uint8_t key(const std::string &k, const uint8_t default_value) const override { return key<uint8_t>(k, default_value); }
	virtual uint16_t key(const std::string &k, const uint16_t default_value) const override { return key<uint16_t>(k, default_value); }
	virtual uint32_t key(const std::string &k, const uint32_t default_value) const override { return key<uint32_t>(k, default_value); }
	virtual uint64_t key(const std::string &k, const uint64_t default_value) const override { return key<uint64_t>(k, default_value); }
	virtual int8_t key(const std::string &k, const int8_t default_value) const override { return key<int8_t>(k, default_value); }
	virtual int16_t key(const std::string &k, const int16_t default_value) const override { return key<int16_t>(k, default_value); }
	virtual int32_t key(const std::string &k, const int32_t default_value) const override { return key<int32_t>(k, default_value); }
	virtual int64_t key(const std::string &k, const int64_t default_value) const override { return key<int64_t>(k, default_value); }

	virtual void set(const std::string &k, const std::string &v, const std::string &src) override { set_as<std::string>(k, v, src); }
	virtual void set(const std::string &k, float v, const std::string &src) override { set_as<float>(k, v, src); }
	virtual void set(const std::string &k, uint8_t v, const std::string &src) override { set_as<uint8_t>(k, v, src); }
	virtual void set(const std::string &k, uint16_t v, const std::string &src) override { set_as<uint16_t>(k, v, src); }
	virtual void set(const std::string &k, uint32_t v, const std::string &src) override { set_as<uint32_t>(k, v, src); }
	virtual void set(const std::string &k, uint64_t v, const std::string &src) override { set_as<uint64_t>(k, v, src); }
	virtual void set(const std::string &k, int8_t v, const std::string &src) override { set_as<int8_t>(k, v, src); }
	virtual void set(const std::string &k, int16_t v, const std::string &src) override { set_as<int16_t>(k, v, src); }
	virtual void set(const std::string &k, int32_t v, const std::string &src) override { set_as<int32_t>(k, v, src); }
	virtual void set(const std::string &k, int64_t v, const std::string &src) override { set_as<int64_t>(k, v, src); }

	/**
	 * Returns the current value for the given key.
	 */
	template<typename T>
	T key(const std::string &k, const T default_value) const
	{
		std::lock_guard<std::mutex> guard(mutex_);
		if(strict_mode_ && defaults_.count(k) == 0) {
			throw std::runtime_error("config key [" + k + "] does not exist");
		}

		try {
			if(defaults_.cend() == defaults_.find(k)) {
				auto v = to_string(default_value);
				defaults_[k] = default_value;
			} else if(boost::get<T>(defaults_[k]) != default_value) {
				auto v = to_string(default_value);
				ERROR << "Mismatched default value for config key [" << k << "], requested " << v << " but previously had " << current_info<T>(k);
			}
			if(current_.cend() == current_.find(k)) {
				apply<T>(k, default_value, "default");
			}
			auto v = boost::get<T>(std::get<0>(current_[k]));
			return v;
		} catch(const boost::bad_get &ex) {
			ERROR << "Failed to get config value, this is probably a type mismatch: " << k;
			ERROR << "Returning default value for " << k << " as a last resort";
			return default_value;
		}
	}

protected:
	template<typename T>
	void set_as(const std::string &k, const T v, const std::string &src = "unknown")
	{
		boost::any prev;
		try {
			prev = boost::any { boost::get<T>(std::get<0>(current_[k])) };
		} catch(const boost::bad_get &ex) {
			/* This is fine, just means we had no previous value */
			prev = boost::any { T() };
		}

		{
			std::lock_guard<std::mutex> guard(mutex_);
			apply<T>(k, v, src);
		}
		if(watchers_.count(k) > 0) {
			boost::any curr;
			try {
				curr = boost::any { boost::get<T>(std::get<0>(current_[k])) };
				for(auto &code : *(watchers_[k])) {
					TRACE << "Notifying watcher for new config value on " << k;
					code(curr, prev);
				}
			} catch(const boost::bad_get &ex) {
				ERROR << "Unable to get previous value in set: " << ex.what();
			}
		}
	}

	/**
	 * Sets a new value for the given key.
	 * We're const because we support being called when applying defaults.
	 * (all relevant variables are mutable)
	 */
	template<typename T>
	void apply(const std::string &k, const T v, const std::string &src = "unknown") const
	{
		current_[k] = std::make_tuple(
			storage_type { v },
			src,
			boost::chrono::high_resolution_clock::now()
		);
	}

	template<typename T>
	std::string current_info(const std::string &k) const
	{
		std::string src;
		storage_type v;
		boost::chrono::high_resolution_clock::time_point t;
		std::tie(v, src, t) = current_.at(k);

		std::stringstream s;
		s << to_string(boost::get<T>(v)) << " (set by " << src << " at " << boost::chrono::time_fmt(boost::chrono::timezone::utc, "%Y-%m-%d %H:%M:%S") << t << ")";
		return s.str();
	}

	template<typename T>
	void
	add_option(const std::string &k, T def, std::string desc) {
		namespace po = boost::program_options;
		{
			std::lock_guard<std::mutex> guard(mutex_);
			if(defaults_.count(k) > 0) {
				ERROR << "Attempting to add config key [" << k << "] more than once, previous description: " << description_[k];
				return;
			}
			description_[k] = desc;
			defaults_[k] = def;
		}

		set(k, def, "definition");
		options_desc_.add_options()
			/* boost::po seems to be allergic to strings... */
			(
				static_cast<const char *>(k.data()),
				po::value<T>(),
				static_cast<const char *>(desc.data())
			)
		;
	}

	void apply_from_vm(boost::program_options::variables_map &vm)
	{
		std::string src { "unknown" };
		for(auto &v : vm) {
			//std::cout << "vm entry: " << v.first << "\n";
			if(!v.second.empty()) {
				auto s = visitor_(v.first, v.second.value(), src);
				DEBUG << "Applying config [" << v.first << "] = " << s;
				current_as_string_[v.first] = s;
			}
		}
	}

	template<typename T>
	std::shared_ptr<watcher>
	watch_as(const std::string &k, std::function<void(T, T)> code) const {
		std::lock_guard<std::mutex> guard(mutex_);
		if(watchers_.count(k) == 0) {
			watchers_[k] = std::unique_ptr<
				std::vector<
					std::function<
						void(boost::any&, boost::any&)
					>
				>
			>(new std::vector<
				std::function<
					void(boost::any&, boost::any&)
				>
			> { });
		}
		auto entry = [code](boost::any &curr, boost::any &prev) {
			T old = boost::any_cast<T&>(prev);
			T v = boost::any_cast<T&>(curr);
			code(v, old);
		};
		watchers_[k]->push_back(entry);
		return std::make_shared<watcher>();
	}

	void apply_environment(const std::string &prefix) {
		namespace po = boost::program_options;
		po::variables_map vm;
		po::store(
			po::parse_environment(
				options_desc_,
				[&prefix](const std::string &in) -> std::string {
					if(in.compare(0, prefix.size(), prefix) == 0) {
						// Skip trailing _ as well
						auto key = in.substr(prefix.size() + 1);
						boost::algorithm::to_lower(key);
						return key;
					}
					return "";
				}
			),
			vm
		);
		apply_from_vm(vm);
	}

	void apply_args(int argc, const char *argv[]) {
		namespace po = boost::program_options;
		po::variables_map vm;
		po::store(
			po::command_line_parser(
				argc, argv
			).options(options_desc_)
			 .allow_unregistered()
			 .run(),
			vm
		);
		apply_from_vm(vm);
	}

	void apply_file(const std::string &path) {
		namespace po = boost::program_options;
		po::variables_map vm;
		if(boost::filesystem::exists(path)) {
			DEBUG << "Loading config from file [" << path << "]";
			po::store(
				po::parse_config_file<char>(
					path.data(),
					options_desc_,
					true // allow unregistered
				),
				vm
			);
		} else {
			INFO << "File [" << path << "] not found, skipping config";
		}
		apply_from_vm(vm);
	}

private:
	/** Strict mode means that we don't accept unknown key requests */
	bool strict_mode_;
	/** Guard access across threads */
	mutable std::mutex mutex_;
	/** Used for type iteration */
	any_visitor visitor_;
	/** Handler that glues type iterator to the config update call */
	handler handler_;
	/** Boost program_options descriptor */
	boost::program_options::options_description options_desc_;
	/** Default values for the known config entries */
	mutable std::unordered_map<std::string, storage_type> defaults_;
	/** Current values for keys */
	mutable std::unordered_map<
		std::string, // key
		current_type
	> current_;
	mutable std::map<
		std::string, // key
		std::string
	> current_as_string_;
	/** Key descriptions */
	std::unordered_map<
		std::string, // key
		std::string
	> description_;

	/** Watchers for values */
	mutable std::unordered_map<
		std::string,
		std::unique_ptr<
			std::vector<
				std::function<void(boost::any&, boost::any&)>
			>
		>
	> watchers_;
	std::vector<
		std::function<void()>
	> loaders_;
};
};
};
