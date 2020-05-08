#include "catch.hpp"

#include <iterator>

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/tag.hpp>

TEST_CASE("Operators") {

    SECTION("Equal") {
        osrm_osmium::memory::Buffer buffer1(10240);
        {
            osrm_osmium::builder::TagListBuilder tl_builder(buffer1);
            tl_builder.add_tag("highway", "primary");
            tl_builder.add_tag("name", "Main Street");
            tl_builder.add_tag("source", "GPS");
        }
        buffer1.commit();

        osrm_osmium::memory::Buffer buffer2(10240);
        {
            osrm_osmium::builder::TagListBuilder tl_builder(buffer2);
            tl_builder.add_tag("highway", "primary");
        }
        buffer2.commit();

        const osrm_osmium::TagList& tl1 = buffer1.get<const osrm_osmium::TagList>(0);
        const osrm_osmium::TagList& tl2 = buffer2.get<const osrm_osmium::TagList>(0);

        auto tagit1 = tl1.begin();
        auto tagit2 = tl2.begin();
        REQUIRE(*tagit1 == *tagit2);
        ++tagit1;
        REQUIRE(!(*tagit1 == *tagit2));
    }

    SECTION("Order") {
        osrm_osmium::memory::Buffer buffer(10240);
        {
            osrm_osmium::builder::TagListBuilder tl_builder(buffer);
            tl_builder.add_tag("highway", "residential");
            tl_builder.add_tag("highway", "primary");
            tl_builder.add_tag("name", "Main Street");
            tl_builder.add_tag("amenity", "post_box");
        }
        buffer.commit();

        const osrm_osmium::TagList& tl = buffer.get<const osrm_osmium::TagList>(0);
        const osrm_osmium::Tag& t1 = *(tl.begin());
        const osrm_osmium::Tag& t2 = *(std::next(tl.begin(), 1));
        const osrm_osmium::Tag& t3 = *(std::next(tl.begin(), 2));
        const osrm_osmium::Tag& t4 = *(std::next(tl.begin(), 3));

        REQUIRE(t2 < t1);
        REQUIRE(t1 < t3);
        REQUIRE(t2 < t3);
        REQUIRE(t4 < t1);
    }

}
