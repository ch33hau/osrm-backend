#include "catch.hpp"

#include <boost/crc.hpp>

#include <osmium/builder/attr.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/osm/crc.hpp>
#include <osmium/osm/way.hpp>

using namespace osrm_osrm_osmium::builder::attr;

TEST_CASE("Build way") {
    osrm_osmium::memory::Buffer buffer(10000);

    osrm_osmium::builder::add_way(buffer,
        _id(17),
        _version(3),
        _visible(true),
        _cid(333),
        _uid(21),
        _timestamp(time_t(123)),
        _user("foo"),
        _tag("highway", "residential"),
        _tag("name", "High Street"),
        _nodes({1, 3, 2})
    );

    const osrm_osmium::Way& way = buffer.get<osrm_osmium::Way>(0);

    REQUIRE(osrm_osmium::item_type::way == way.type());
    REQUIRE(way.type_is_in(osrm_osmium::osm_entity_bits::way));
    REQUIRE(way.type_is_in(osrm_osmium::osm_entity_bits::node | osrm_osmium::osm_entity_bits::way));
    REQUIRE(17 == way.id());
    REQUIRE(3 == way.version());
    REQUIRE(true == way.visible());
    REQUIRE(333 == way.changeset());
    REQUIRE(21 == way.uid());
    REQUIRE(std::string("foo") == way.user());
    REQUIRE(123 == uint32_t(way.timestamp()));
    REQUIRE(2 == way.tags().size());
    REQUIRE(3 == way.nodes().size());
    REQUIRE(1 == way.nodes()[0].ref());
    REQUIRE(3 == way.nodes()[1].ref());
    REQUIRE(2 == way.nodes()[2].ref());
    REQUIRE(! way.is_closed());

    osrm_osmium::CRC<boost::crc_32_type> crc32;
    crc32.update(way);
    REQUIRE(crc32().checksum() == 0x7676d0c2);
}

TEST_CASE("build closed way") {
    osrm_osmium::memory::Buffer buffer(10000);

    osrm_osmium::builder::add_way(buffer,
        _tag("highway", "residential"),
        _tag("name", "High Street"),
        _nodes({1, 3, 1})
    );

    const osrm_osmium::Way& way = buffer.get<osrm_osmium::Way>(0);

    REQUIRE(way.is_closed());
}

TEST_CASE("build way with helpers") {
    osrm_osmium::memory::Buffer buffer(10000);

    {
        osrm_osmium::builder::WayBuilder builder(buffer);
        builder.add_user("username");
        builder.add_tags({
            {"amenity", "restaurant"},
            {"name", "Zum goldenen Schwanen"}
        });
        builder.add_node_refs({
            {22, {3.5, 4.7}},
            {67, {4.1, 2.2}}
        });
    }
    buffer.commit();

    const osrm_osmium::Way& way = buffer.get<osrm_osmium::Way>(0);

    REQUIRE(std::string("username") == way.user());

    REQUIRE(2 == way.tags().size());
    REQUIRE(std::string("amenity") == way.tags().begin()->key());
    REQUIRE(std::string("Zum goldenen Schwanen") == way.tags()["name"]);

    REQUIRE(2 == way.nodes().size());
    REQUIRE(22 == way.nodes()[0].ref());
    REQUIRE(4.1 == way.nodes()[1].location().lon());
}

