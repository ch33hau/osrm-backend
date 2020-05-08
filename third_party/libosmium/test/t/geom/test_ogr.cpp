#include "catch.hpp"

#include <osmium/geom/ogr.hpp>

#include "area_helper.hpp"
#include "wnl_helper.hpp"

TEST_CASE("OGR_Geometry") {

SECTION("point") {
    osrm_osmium::geom::OGRFactory<> factory;

    std::unique_ptr<OGRPoint> point {factory.create_point(osrm_osmium::Location(3.2, 4.2))};
    REQUIRE(3.2 == point->getX());
    REQUIRE(4.2 == point->getY());
}

SECTION("empty_point") {
    osrm_osmium::geom::OGRFactory<> factory;

    REQUIRE_THROWS_AS(factory.create_point(osrm_osmium::Location()), osrm_osmium::invalid_location);
}

SECTION("linestring") {
    osrm_osmium::geom::OGRFactory<> factory;

    osrm_osmium::memory::Buffer buffer(10000);
    auto &wnl = create_test_wnl_okay(buffer);

    {
        std::unique_ptr<OGRLineString> linestring {factory.create_linestring(wnl)};
        REQUIRE(3 == linestring->getNumPoints());

        REQUIRE(3.2 == linestring->getX(0));
        REQUIRE(3.6 == linestring->getX(2));
    }

    {
        std::unique_ptr<OGRLineString> linestring {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::unique, osrm_osmium::geom::direction::backward)};
        REQUIRE(3 == linestring->getNumPoints());

        REQUIRE(3.6 == linestring->getX(0));
        REQUIRE(3.2 == linestring->getX(2));
    }

    {
        std::unique_ptr<OGRLineString> linestring {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all)};
        REQUIRE(4 == linestring->getNumPoints());

        REQUIRE(3.2 == linestring->getX(0));
    }

    {
        std::unique_ptr<OGRLineString> linestring {factory.create_linestring(wnl, osrm_osmium::geom::use_nodes::all, osrm_osmium::geom::direction::backward)};
        REQUIRE(4 == linestring->getNumPoints());

        REQUIRE(3.6 == linestring->getX(0));
    }
}

SECTION("area_1outer_0inner") {
    osrm_osmium::geom::OGRFactory<> factory;

    osrm_osmium::memory::Buffer buffer(10000);
    const osrm_osmium::Area& area = create_test_area_1outer_0inner(buffer);

    std::unique_ptr<OGRMultiPolygon> mp {factory.create_multipolygon(area)};
    REQUIRE(1 == mp->getNumGeometries());

    const OGRPolygon* p0 = dynamic_cast<const OGRPolygon*>(mp->getGeometryRef(0));
    REQUIRE(0 == p0->getNumInteriorRings());

    const OGRLineString* l0e = p0->getExteriorRing();
    REQUIRE(4 == l0e->getNumPoints());

    REQUIRE(3.5 == l0e->getX(1));
}

SECTION("area_1outer_1inner") {
    osrm_osmium::geom::OGRFactory<> factory;

    osrm_osmium::memory::Buffer buffer(10000);
    const osrm_osmium::Area& area = create_test_area_1outer_1inner(buffer);

    std::unique_ptr<OGRMultiPolygon> mp {factory.create_multipolygon(area)};
    REQUIRE(1 == mp->getNumGeometries());

    const OGRPolygon* p0 = dynamic_cast<const OGRPolygon*>(mp->getGeometryRef(0));
    REQUIRE(1 == p0->getNumInteriorRings());

    const OGRLineString* l0e = p0->getExteriorRing();
    REQUIRE(5 == l0e->getNumPoints());

    const OGRLineString* l0i0 = p0->getInteriorRing(0);
    REQUIRE(5 == l0i0->getNumPoints());
}

SECTION("area_2outer_2inner") {
    osrm_osmium::geom::OGRFactory<> factory;

    osrm_osmium::memory::Buffer buffer(10000);
    const osrm_osmium::Area& area = create_test_area_2outer_2inner(buffer);

    std::unique_ptr<OGRMultiPolygon> mp {factory.create_multipolygon(area)};
    REQUIRE(2 == mp->getNumGeometries());

    const OGRPolygon* p0 = dynamic_cast<const OGRPolygon*>(mp->getGeometryRef(0));
    REQUIRE(2 == p0->getNumInteriorRings());

    const OGRLineString* l0e = p0->getExteriorRing();
    REQUIRE(5 == l0e->getNumPoints());

    const OGRPolygon* p1 = dynamic_cast<const OGRPolygon*>(mp->getGeometryRef(1));
    REQUIRE(0 == p1->getNumInteriorRings());

    const OGRLineString* l1e = p1->getExteriorRing();
    REQUIRE(5 == l1e->getNumPoints());
}

}

