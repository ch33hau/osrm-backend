#include "catch.hpp"

#include <osmium/builder/attr.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/node_ref.hpp>
#include <osmium/osm/node_ref_list.hpp>

TEST_CASE("NodeRef") {

    SECTION("instantiation_with_default_parameters") {
        osrm_osmium::NodeRef node_ref;
        REQUIRE(node_ref.ref() == 0);
//    REQUIRE(!node_ref.has_location());
    }

    SECTION("instantiation_with_id") {
        osrm_osmium::NodeRef node_ref(7);
        REQUIRE(node_ref.ref() == 7);
    }

    SECTION("equality") {
        osrm_osmium::NodeRef node_ref1(7, { 1.2, 3.4 });
        osrm_osmium::NodeRef node_ref2(7, { 1.4, 3.1 });
        osrm_osmium::NodeRef node_ref3(9, { 1.2, 3.4 });
        REQUIRE(node_ref1 == node_ref2);
        REQUIRE(node_ref1 != node_ref3);
        REQUIRE(!osrm_osmium::location_equal()(node_ref1, node_ref2));
        REQUIRE(!osrm_osmium::location_equal()(node_ref2, node_ref3));
        REQUIRE(osrm_osmium::location_equal()(node_ref1, node_ref3));
    }

    SECTION("set_location") {
        osrm_osmium::NodeRef node_ref(7);
        REQUIRE(!node_ref.location().valid());
        REQUIRE(node_ref.location() == osrm_osmium::Location());
        node_ref.set_location(osrm_osmium::Location(13.5, -7.2));
        REQUIRE(node_ref.location().lon() == 13.5);
        REQUIRE(node_ref.location().valid());
    }

    SECTION("ordering") {
        osrm_osmium::NodeRef node_ref1(1, { 1.0, 3.0 });
        osrm_osmium::NodeRef node_ref2(2, { 1.4, 2.9 });
        osrm_osmium::NodeRef node_ref3(3, { 1.2, 3.0 });
        osrm_osmium::NodeRef node_ref4(4, { 1.2, 3.3 });

        REQUIRE(node_ref1 < node_ref2);
        REQUIRE(node_ref2 < node_ref3);
        REQUIRE(node_ref1 < node_ref3);
        REQUIRE(node_ref1 >= node_ref1);

        REQUIRE(osrm_osmium::location_less()(node_ref1, node_ref2));
        REQUIRE(!osrm_osmium::location_less()(node_ref2, node_ref3));
        REQUIRE(osrm_osmium::location_less()(node_ref1, node_ref3));
        REQUIRE(osrm_osmium::location_less()(node_ref3, node_ref4));
        REQUIRE(!osrm_osmium::location_less()(node_ref1, node_ref1));
    }

}

TEST_CASE("WayNodeList") {
    osrm_osmium::memory::Buffer buffer(1024);

    SECTION("Empty list") {
        {
            osrm_osmium::builder::WayNodeListBuilder builder(buffer);
        }
        REQUIRE(buffer.commit() == 0);
        REQUIRE(buffer.committed( )> 0);

        const osrm_osmium::WayNodeList& nrl = buffer.get<osrm_osmium::WayNodeList>(0);
        REQUIRE(nrl.empty());
        REQUIRE(nrl.size() == 0);
    }

    SECTION("Small area") {
        osrm_osmium::builder::add_way_node_list(buffer, osrm_osmium::builder::attr::_nodes({
            { 1, {0, 0}},
            { 2, {0, 1}},
            { 3, {1, 1}},
            { 4, {1, 0}},
            { 1, {0, 0}},
        }));

        const osrm_osmium::WayNodeList& nrl = buffer.get<osrm_osmium::WayNodeList>(0);
        REQUIRE_FALSE(nrl.empty());
        REQUIRE(nrl.size() == 5);
        REQUIRE(nrl.is_closed());
        REQUIRE(nrl.ends_have_same_id());
        REQUIRE(nrl.ends_have_same_location());
    }

    SECTION("Not an area") {
        osrm_osmium::builder::add_way_node_list(buffer, osrm_osmium::builder::attr::_nodes({
            { 1, {0, 0}},
            { 2, {1, 0}},
            { 1, {0, 0}},
        }));

        const osrm_osmium::WayNodeList& nrl = buffer.get<osrm_osmium::WayNodeList>(0);
        REQUIRE_FALSE(nrl.empty());
        REQUIRE(nrl.size() == 3);
        REQUIRE(nrl.is_closed());
        REQUIRE(nrl.ends_have_same_id());
        REQUIRE(nrl.ends_have_same_location());
    }

}

