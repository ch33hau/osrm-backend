#include "catch.hpp"

#include <osmium/geom/geos.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/projection.hpp>
#include <osmium/geom/wkb.hpp>
#include <osmium/geom/wkt.hpp>

#include "helper.hpp"

TEST_CASE("Projection") {

    SECTION("point_mercator") {
        osrm_osmium::geom::WKTFactory<osrm_osmium::geom::MercatorProjection> factory(2);

        std::string wkt {factory.create_point(osrm_osmium::Location(3.2, 4.2))};
        REQUIRE(std::string {"POINT(356222.37 467961.14)"} == wkt);
    }

    SECTION("point_epsg_3857") {
        osrm_osmium::geom::WKTFactory<osrm_osmium::geom::Projection> factory(osrm_osmium::geom::Projection(3857), 2);

        std::string wkt {factory.create_point(osrm_osmium::Location(3.2, 4.2))};
        REQUIRE(std::string {"POINT(356222.37 467961.14)"} == wkt);
    }

    SECTION("wkb_with_parameter") {
        osrm_osmium::geom::WKBFactory<osrm_osmium::geom::Projection> wkb_factory(osrm_osmium::geom::Projection(3857), osrm_osmium::geom::wkb_type::wkb, osrm_osmium::geom::out_type::hex);
        osrm_osmium::geom::GEOSFactory<osrm_osmium::geom::Projection> geos_factory(osrm_osmium::geom::Projection(3857));

        std::string wkb = wkb_factory.create_point(osrm_osmium::Location(3.2, 4.2));
        std::unique_ptr<geos::geom::Point> geos_point = geos_factory.create_point(osrm_osmium::Location(3.2, 4.2));
        REQUIRE(geos_to_wkb(geos_point.get()) == wkb);
    }

    SECTION("cleanup") {
        // trying to make valgrind happy, but there is still a memory leak in proj library
        pj_deallocate_grids();
    }

}
