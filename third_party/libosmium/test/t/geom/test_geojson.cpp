#include "catch.hpp"

#include <osmium/geom/geojson.hpp>

#include "area_helper.hpp"
#include "wnl_helper.hpp"

TEST_CASE("GeoJSON_Geometry") {

SECTION("point") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    std::string json {factory.create_point(osrm_osmium::Location(3.2, 4.2))};
    REQUIRE(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"} == json);
}

SECTION("empty_point") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    REQUIRE_THROWS_AS(factory.create_point(osrm_osmium::Location()), osrm_osmium::invalid_location);
}

SECTION("linestring") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    auto &wnl = create_test_wnl_okay(buffer);

    {
        std::string json {factory.create_linestring(wnl)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    {
        std::string json {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::unique, osrm_osmium::geom::direction::backward)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.2,4.2]]}"} == json);
    }

    {
        std::string json {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.5,4.7],[3.6,4.9]]}"} == json);
    }

    {
        std::string json {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all, osrm_osmium::geom::direction::backward)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.5,4.7],[3.2,4.2]]}"} == json);
    }
}

SECTION("empty_linestring") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    auto& wnl = create_test_wnl_empty(buffer);

    REQUIRE_THROWS_AS(factory.create_linestring(wnl), osrm_osmium::geometry_error);
    REQUIRE_THROWS_AS(factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::unique, osrm_osmium::geom::direction::backward), osrm_osmium::geometry_error);
    REQUIRE_THROWS_AS(factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all), osrm_osmium::geometry_error);
    REQUIRE_THROWS_AS(factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all, osrm_osmium::geom::direction::backward), osrm_osmium::geometry_error);
}

SECTION("linestring_with_two_same_locations") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    auto& wnl = create_test_wnl_same_location(buffer);

    REQUIRE_THROWS_AS(factory.create_linestring(wnl), osrm_osmium::geometry_error);
    REQUIRE_THROWS_AS(factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::unique, osrm_osmium::geom::direction::backward), osrm_osmium::geometry_error);

    {
        std::string json {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"} == json);
    }

    {
        std::string json {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all, osrm_osmium::geom::direction::backward)};
        REQUIRE(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"} == json);
    }
}

SECTION("linestring_with_undefined_location") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    auto& wnl = create_test_wnl_undefined_location(buffer);

    REQUIRE_THROWS_AS(factory.create_linestring(wnl), osrm_osmium::invalid_location);
}

SECTION("area_1outer_0inner") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    const osrm_osmium::Area& area = create_test_area_1outer_0inner(buffer);

    REQUIRE(!area.is_multipolygon());
    REQUIRE(std::distance(area.cbegin(), area.cend()) == 2);
    REQUIRE(std::distance(area.cbegin<osrm_osmium::OuterRing>(), area.cend<osrm_osmium::OuterRing>()) == area.num_rings().first);

    {
        std::string json {factory.create_multipolygon(area)};
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[3.2,4.2],[3.5,4.7],[3.6,4.9],[3.2,4.2]]]]}"} == json);
    }
}

SECTION("area_1outer_1inner") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    const osrm_osmium::Area& area = create_test_area_1outer_1inner(buffer);

    REQUIRE(!area.is_multipolygon());
    REQUIRE(std::distance(area.cbegin(), area.cend()) == 3);
    REQUIRE(std::distance(area.cbegin<osrm_osmium::OuterRing>(), area.cend<osrm_osmium::OuterRing>()) == area.num_rings().first);
    REQUIRE(std::distance(area.cbegin<osrm_osmium::InnerRing>(), area.cend<osrm_osmium::InnerRing>()) == area.num_rings().second);

    {
        std::string json {factory.create_multipolygon(area)};
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1,1],[8,1],[8,8],[1,8],[1,1]]]]}"} == json);
    }
}

SECTION("area_2outer_2inner") {
    osrm_osmium::geom::GeoJSONFactory<> factory;

    osrm_osmium::memory::Buffer buffer(1000);
    const osrm_osmium::Area& area = create_test_area_2outer_2inner(buffer);

    REQUIRE(area.is_multipolygon());
    REQUIRE(std::distance(area.cbegin(), area.cend()) == 5);
    REQUIRE(std::distance(area.cbegin<osrm_osmium::OuterRing>(), area.cend<osrm_osmium::OuterRing>()) == area.num_rings().first);
    REQUIRE(std::distance(area.cbegin<osrm_osmium::InnerRing>(), area.cend<osrm_osmium::InnerRing>()) == area.num_rings().second);

    int outer_ring=0;
    int inner_ring=0;
    for (auto it_outer = area.cbegin<osrm_osmium::OuterRing>(); it_outer !=  area.cend<osrm_osmium::OuterRing>(); ++it_outer) {
        if (outer_ring == 0) {
            REQUIRE(it_outer->front().ref() == 1);
        } else if (outer_ring == 1) {
            REQUIRE(it_outer->front().ref() == 100);
        } else {
            REQUIRE(false);
        }
        for (auto it_inner = area.inner_ring_cbegin(it_outer); it_inner != area.inner_ring_cend(it_outer); ++it_inner) {
            if (outer_ring == 0 && inner_ring == 0) {
                REQUIRE(it_inner->front().ref() == 5);
            } else if (outer_ring == 0 && inner_ring == 1) {
                REQUIRE(it_inner->front().ref() == 10);
            } else {
                REQUIRE(false);
            }
            ++inner_ring;
        }
        inner_ring = 0;
        ++outer_ring;
    }

    {
        std::string json {factory.create_multipolygon(area)};
        REQUIRE(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1,1],[4,1],[4,4],[1,4],[1,1]],[[5,5],[5,7],[7,7],[5,5]]],[[[10,10],[11,10],[11,11],[10,11],[10,10]]]]}"} == json);
    }
}

}

