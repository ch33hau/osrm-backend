
#include "catch.hpp"

#include <osmium/util/memory.hpp>

TEST_CASE("Check memory usage") {
#ifdef __linux__
    const int size_in_mbytes = 10;

    osrm_osmium::MemoryUsage m1;
    REQUIRE(m1.current() > 1);
    REQUIRE(m1.peak() > 1);

    {
        std::vector<int> v;
        v.reserve(size_in_mbytes * 1024 * 1024);

        osrm_osmium::MemoryUsage m2;
        REQUIRE(m2.current() >= m1.current() + size_in_mbytes);
        REQUIRE(m2.peak() >= m1.peak() + size_in_mbytes);
        REQUIRE(m2.peak() - m2.current() <= 1);
    }

    osrm_osmium::MemoryUsage m3;
    REQUIRE(m3.current() > 1);
    REQUIRE(m3.current() < m3.peak());
    REQUIRE(m3.peak() >= m1.peak() + size_in_mbytes);
#else
    osrm_osmium::MemoryUsage m;
    REQUIRE(m.current() == 0);
    REQUIRE(m.peak() == 0);
#endif
}

