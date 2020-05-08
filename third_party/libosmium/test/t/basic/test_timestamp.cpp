#include "catch.hpp"

#include <sstream>

#include <osmium/osm/timestamp.hpp>

TEST_CASE("Timestamp") {

    SECTION("can be default initialized to invalid value") {
        osrm_osmium::Timestamp t;
        REQUIRE(0 == uint32_t(t));
        REQUIRE("" == t.to_iso());
        REQUIRE_FALSE(t.valid());
    }

    SECTION("invalid value is zero") {
        osrm_osmium::Timestamp t(static_cast<time_t>(0));
        REQUIRE(0 == uint32_t(t));
        REQUIRE("" == t.to_iso());
        REQUIRE_FALSE(t.valid());
    }

    SECTION("can be initialized from time_t") {
        osrm_osmium::Timestamp t(static_cast<time_t>(1));
        REQUIRE(1 == uint32_t(t));
        REQUIRE("1970-01-01T00:00:01Z" == t.to_iso());
        REQUIRE(t.valid());
    }

    SECTION("can be initialized from const char*") {
        osrm_osmium::Timestamp t("2000-01-01T00:00:00Z");
        REQUIRE("2000-01-01T00:00:00Z" == t.to_iso());
        REQUIRE(t.valid());
    }

    SECTION("can be initialized from string") {
        std::string s = "2000-01-01T00:00:00Z";
        osrm_osmium::Timestamp t(s);
        REQUIRE("2000-01-01T00:00:00Z" == t.to_iso());
        REQUIRE(t.valid());
    }

    SECTION("throws if initialized from bad string") {
        REQUIRE_THROWS_AS(osrm_osmium::Timestamp("x"), std::invalid_argument);
    }

    SECTION("can be explicitly cast to time_t") {
        osrm_osmium::Timestamp t(4242);
        time_t x = t.seconds_since_epoch();
        REQUIRE(x == 4242);
    }

    SECTION("uint32_t can be initialized from Timestamp") {
        osrm_osmium::Timestamp t(4242);
        uint32_t x { t };

        REQUIRE(x == 4242);
    }

    SECTION("can be compared") {
        osrm_osmium::Timestamp t1(10);
        osrm_osmium::Timestamp t2(50);
        REQUIRE(t1 < t2);
        REQUIRE(t1 > osrm_osmium::start_of_time());
        REQUIRE(t2 > osrm_osmium::start_of_time());
        REQUIRE(t1 < osrm_osmium::end_of_time());
        REQUIRE(t2 < osrm_osmium::end_of_time());
    }

    SECTION("can be written to stream") {
        std::stringstream ss;
        osrm_osmium::Timestamp t(1);
        ss << t;
        REQUIRE("1970-01-01T00:00:01Z" == ss.str());
    }

}
