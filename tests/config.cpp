#include "catch.hpp"
#include <appcon.h>
#include "cfgmaker.h"

using namespace appcon;

SCENARIO("can create config") {
	GIVEN("empty state") {
		WHEN("we instantiate a config object") {
			auto cfg = make_config();
			THEN("config is valid") {
				REQUIRE(cfg);
			}
		}
	}
	GIVEN("config with no settings") {
		auto cfg = make_config();
		REQUIRE(cfg);
		WHEN("we add a string option") {
			REQUIRE_NOTHROW(
				(*cfg)("test", std::string { "default value" }, "a test option")
			);
			THEN("option is valid") {
				REQUIRE(cfg->have_key("test"));
				REQUIRE(cfg->key("test", std::string { "default value" }) == "default value");
				REQUIRE(cfg->description("test") == "a test option");
			}
		}
	}
	GIVEN("config with some settings") {
		auto cfg = make_config();
		REQUIRE(cfg);
		REQUIRE_NOTHROW(
			(*cfg)("test", std::string { "default value" }, "a test option")
		);
		WHEN("we use a missing option without strict") {
			cfg->strict(false);
			THEN("we add the entry automatically") {
				REQUIRE_NOTHROW(cfg->key("missing", std::string { "some mising entry" }));
				CHECK(cfg->have_key("missing"));
			}
		}
		WHEN("we use a missing option with strict") {
			cfg->strict(true);
			THEN("we get an exception") {
				REQUIRE_THROWS(cfg->key("missing", std::string { "some mising entry" }));
				CHECK(!cfg->have_key("missing"));
			}
		}
	}
}

