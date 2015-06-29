		/*
#include "catch.hpp"
#include <appcon.h>
#include "cfgmaker.h"

using namespace appcon;

SCENARIO("config from environment") {
	GIVEN("a config object") {
		auto cfg = make_config();
		WHEN("we apply a single commandline option") {
			cfg->strict(false);
			(*cfg)
				("key", std::string { "default" }, "first key")
			;
			const char *argv[] {
				"test",
				"--key=value"
			};
			int argc = 2;
			REQUIRE_NOTHROW(cfg->from_args(argc, argv));
			THEN("config is valid") {
				CHECK(cfg->have_key("key"));
				CHECK(cfg->key("key", std::string { "x" }) == "value");
				CHECK(!cfg->have_key("test"));
			}
		}
		WHEN("we apply multiple commandline options") {
			cfg->strict(true);
			(*cfg)
				("key", std::string { "default" }, "first key")
				("second", uint16_t { 0 }, "second key")
			;
			const char *argv[] {
				"test",
				"--key=value",
				"--second=123"
			};
			int argc = 3;
			REQUIRE_NOTHROW(cfg->from_args(argc, argv));
			THEN("config is valid") {
				CHECK(cfg->have_key("key"));
				CHECK(cfg->key("key", std::string { "default" }) == "value");
				CHECK(cfg->have_key("second"));
				CHECK(cfg->key("second", uint16_t { 0 }) == 123);
			}
		}
	}
}

		*/
