#include "catch.hpp"

#include <osmium/osm/area.hpp>

TEST_CASE("area_id") {

    SECTION("object_id_to_area_id_conversion") {
        REQUIRE( 46 == osrm_osmium::object_id_to_area_id( 23, osrm_osmium::item_type::way));
        REQUIRE( 47 == osrm_osmium::object_id_to_area_id( 23, osrm_osmium::item_type::relation));
        REQUIRE(  0 == osrm_osmium::object_id_to_area_id(  0, osrm_osmium::item_type::way));
        REQUIRE(  1 == osrm_osmium::object_id_to_area_id(  0, osrm_osmium::item_type::relation));
        REQUIRE(-24 == osrm_osmium::object_id_to_area_id(-12, osrm_osmium::item_type::way));
        REQUIRE(-25 == osrm_osmium::object_id_to_area_id(-12, osrm_osmium::item_type::relation));
    }

    SECTION("area_id_to_object_id_conversion") {
        REQUIRE( 23 == osrm_osmium::area_id_to_object_id( 46));
        REQUIRE( 23 == osrm_osmium::area_id_to_object_id( 47));
        REQUIRE(  0 == osrm_osmium::area_id_to_object_id(  0));
        REQUIRE(  0 == osrm_osmium::area_id_to_object_id(  1));
        REQUIRE(-12 == osrm_osmium::area_id_to_object_id(-24));
        REQUIRE(-12 == osrm_osmium::area_id_to_object_id(-25));
    }

}
