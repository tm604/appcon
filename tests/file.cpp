/**
 * @file
 */
#include "catch.hpp"
#include <iostream>
#include <fstream>
#include <appcon.h>
#include "cfgmaker.h"

using namespace appcon;

SCENARIO("config from file", "[file]") {
	GIVEN("a config object") {
		auto cfg = make_config();
		WHEN("we apply a single option from a file") {
			cfg->strict(true);
			std::string filename { "config-test.ini" };
			{
				std::ofstream out { filename, std::ios::out | std::ios::binary };
				out << "fromfile = some value\n";
			}
			(*cfg)
				("fromfile", std::string { "default" }, "first key")
			;
			REQUIRE_NOTHROW(cfg->from_file("config-test.ini"));
			THEN("we have that key in config") {
				CHECK(cfg->have_key("fromfile"));
				CHECK(cfg->key("fromfile", std::string { "default" }) == "some value");
			}
		}
		WHEN("we try to read several options from a file") {
			cfg->strict(true);
			std::string filename { "config-test.ini" };
			{
				std::ofstream out { filename, std::ios::out | std::ios::binary };
				out << "string_key = this is a string\n";
				out << "int_key = 45\n";
				out << "float_key = 27.845\n";
			}
			(*cfg)
				("string_key", std::string { "default" }, "first key")
				("int_key", uint32_t { 0 }, "first key")
				("float_key", float { 0.0f }, "first key")
			;
			REQUIRE_NOTHROW(cfg->from_file("config-test.ini"));
			THEN("we have those keys in config") {
				CHECK(cfg->have_key("string_key"));
				CHECK(cfg->key("string_key", std::string { "default" }) == "this is a string");
				CHECK(cfg->have_key("int_key"));
				CHECK(cfg->key("int_key", uint32_t { 0 }) == 45);
				CHECK(cfg->have_key("float_key"));
				CHECK(cfg->key("float_key", float { 0.0f }) == 27.845f);
			}
		}
	}
	GIVEN("a config object with a watcher") {
		auto cfg = make_config();
		cfg->strict(true);
		(*cfg)
			("watched_key", std::string { "default" }, "a key we want to watch for changes")
		;
		REQUIRE(cfg->have_key("watched_key"));
		cfg->set("watched_key", std::string { "original value" }, "manual");
		bool changed = false;
		cfg->watch("watched_key", std::string { "" }, [&](std::string v, std::string old) {
			changed = true;
			CHECK(old == "original value");
			CHECK(v == "updated value");
		});
		REQUIRE(!changed);
		WHEN("we load the initial file") {
			std::string filename { "config-test.ini" };
			{
				std::ofstream out { filename, std::ios::out | std::ios::binary };
				out << "watched_key = updated value\n";
			}
			REQUIRE_NOTHROW(cfg->from_file("config-test.ini"));
			THEN("our watcher was called") {
				CHECK(changed);
				CHECK(cfg->key("watched_key", std::string { "default" }) == "updated value");
			}
		}
	}
	GIVEN("a config object with a watcher") {
		auto cfg = make_config();
		cfg->strict(true);
		(*cfg)
			("watched_key", std::string { "default" }, "a key we want to watch for changes")
		;
		REQUIRE(cfg->have_key("watched_key"));
		std::string filename { "config-test.ini" };
		bool changed = false;
		{
			std::ofstream out { filename, std::ios::out | std::ios::binary };
			out << "watched_key = original value\n";
		}
		REQUIRE_NOTHROW(cfg->from_file("config-test.ini"));
		cfg->watch("watched_key", std::string { "" }, [&](std::string v, std::string old) {
			changed = true;
			CHECK(old == "original value");
			CHECK(v == "updated value");
		});
		REQUIRE(!changed);
		WHEN("we reload the config") {
			{
				std::ofstream out { filename, std::ios::out | std::ios::binary };
				out << "watched_key = updated value\n";
			}
			REQUIRE(cfg->key("watched_key", std::string { "default" }) == "original value");
			REQUIRE_NOTHROW(cfg->reload());
			THEN("our watcher was called") {
				CHECK(changed);
				CHECK(cfg->key("watched_key", std::string { "default" }) == "updated value");
			}
		}
	}
}

