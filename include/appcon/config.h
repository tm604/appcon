/**
 * @file
 */
#pragma once
#include <string>
#include <memory>

namespace appcon {

class watcher {
};

/**
 * Provides an abstraction for dealing with config
 * files.
 */
class config {
public:
	enum {
		/**
		 * Do not complain if we try to load a config file
		 * that does not exist
		 */
		ignore_missing = 0x01
	};

	/** Record a new config entry */
	virtual config &operator()(std::string k, const std::string &def, std::string desc) = 0;
	virtual config &operator()(std::string k, float def, std::string desc) = 0;
	virtual config &operator()(std::string k, uint8_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, uint16_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, uint32_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, uint64_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, int8_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, int16_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, int32_t def, std::string desc) = 0;
	virtual config &operator()(std::string k, int64_t def, std::string desc) = 0;

	/** Indicates that we should also pull data from the environment, with the given prefix */
	virtual config &from_environment(const std::string &prefix) = 0;
	/** Provides commandline data - this will be stored in the config object */
	virtual config &from_args(int argc, const char *argv[]) = 0;
	/** config file */
	virtual config &from_file(const std::string &path) = 0;
	/**
	 * When set, will throw an exception if we try to look up a value that's either not
	 * been defined at all, or has a different default value from the one we have configured.
	 */
	virtual config &strict(bool) = 0;
	/** Do we know this key? */
	virtual bool have_key(std::string k) const = 0;
	/** Description for key */
	virtual const std::string &description(const std::string &k) const = 0;
	/** Watch a config var */
	virtual std::shared_ptr<watcher> watch(const std::string &k, std::function<void(std::string, std::string)> code) const = 0;
	virtual std::shared_ptr<watcher> watch(const std::string &k, std::function<void(float, float)> code) const = 0;
	/** Stop watching */
	virtual config &unwatch(std::shared_ptr<watcher> w) = 0;
	/** Uses local overrides, commandline, environment, config file in that order */
	virtual config &apply() = 0;
	/** Alias for apply() */
	virtual config &reload() = 0;
	/** Iterates through all config values as strings */
	virtual config &each_as_string(std::function<void(std::string, std::string)> code) = 0;

	virtual std::string key(const std::string &k, const std::string default_value) const = 0;
	virtual float key(const std::string &k, const float default_value) const = 0;
	virtual uint8_t key(const std::string &k, const uint8_t default_value) const = 0;
	virtual uint16_t key(const std::string &k, const uint16_t default_value) const = 0;
	virtual uint32_t key(const std::string &k, const uint32_t default_value) const = 0;
	virtual uint64_t key(const std::string &k, const uint64_t default_value) const = 0;
	virtual int8_t key(const std::string &k, const int8_t default_value) const = 0;
	virtual int16_t key(const std::string &k, const int16_t default_value) const = 0;
	virtual int32_t key(const std::string &k, const int32_t default_value) const = 0;
	virtual int64_t key(const std::string &k, const int64_t default_value) const = 0;

	virtual void set(const std::string &k, const std::string &v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const float v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const uint8_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const uint16_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const uint32_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const uint64_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const int8_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const int16_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const int32_t v, const std::string &src = "unknown") = 0;
	virtual void set(const std::string &k, const int64_t v, const std::string &src = "unknown") = 0;
};

};

