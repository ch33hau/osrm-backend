#include "catch.hpp"

#include <random>

#include <osmium/geom/projection.hpp>

TEST_CASE("CRS") {
    osrm_osmium::geom::CRS wgs84{4326};
    osrm_osmium::geom::CRS mercator{3857};

    osrm_osmium::geom::Coordinates c{osrm_osmium::geom::deg_to_rad(1.2), osrm_osmium::geom::deg_to_rad(3.4)};
    auto ct = osrm_osmium::geom::transform(wgs84, mercator, c);
    auto c2 = osrm_osmium::geom::transform(mercator, wgs84, ct);

    REQUIRE(c.x == Approx(c2.x));
    REQUIRE(c.y == Approx(c2.y));
}

