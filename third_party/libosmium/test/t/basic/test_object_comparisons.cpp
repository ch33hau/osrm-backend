#include "catch.hpp"

#include <osmium/builder/attr.hpp>
#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/osm.hpp>
#include <osmium/osm/object_comparisons.hpp>

TEST_CASE("Object_Comparisons") {

    using namespace osrm_osrm_osmium::builder::attr;

    SECTION("order") {
        osrm_osmium::memory::Buffer buffer(10 * 1000);

        osrm_osmium::builder::add_node(buffer, _id(10), _version(1));
        osrm_osmium::builder::add_node(buffer, _id(15), _version(2));

        auto it = buffer.begin();
        osrm_osmium::Node& node1 = static_cast<osrm_osmium::Node&>(*it);
        osrm_osmium::Node& node2 = static_cast<osrm_osmium::Node&>(*(++it));

        REQUIRE(node1 < node2);
        REQUIRE_FALSE(node1 > node2);
        node1.set_id(20);
        node1.set_version(1);
        node2.set_id(20);
        node2.set_version(2);
        REQUIRE(node1 < node2);
        REQUIRE_FALSE(node1 > node2);
        node1.set_id(-10);
        node1.set_version(2);
        node2.set_id(-15);
        node2.set_version(1);
        REQUIRE(node1 < node2);
        REQUIRE_FALSE(node1 > node2);
    }

    SECTION("order_types") {
        osrm_osmium::memory::Buffer buffer(10 * 1000);

        osrm_osmium::builder::add_node(buffer, _id(3), _version(3));
        osrm_osmium::builder::add_node(buffer, _id(3), _version(4));
        osrm_osmium::builder::add_node(buffer, _id(3), _version(4));
        osrm_osmium::builder::add_way(buffer, _id(2), _version(2));
        osrm_osmium::builder::add_relation(buffer, _id(1), _version(1));

        auto it = buffer.begin();
        const osrm_osmium::Node& node1 = static_cast<const osrm_osmium::Node&>(*it);
        const osrm_osmium::Node& node2 = static_cast<const osrm_osmium::Node&>(*(++it));
        const osrm_osmium::Node& node3 = static_cast<const osrm_osmium::Node&>(*(++it));
        const osrm_osmium::Way& way = static_cast<const osrm_osmium::Way&>(*(++it));
        const osrm_osmium::Relation& relation = static_cast<const osrm_osmium::Relation&>(*(++it));

        REQUIRE(node1 < node2);
        REQUIRE(node2 < way);
        REQUIRE_FALSE(node2 > way);
        REQUIRE(way < relation);
        REQUIRE(node1 < relation);

        REQUIRE(osrm_osmium::object_order_type_id_version()(node1, node2));
        REQUIRE(osrm_osmium::object_order_type_id_reverse_version()(node2, node1));
        REQUIRE(osrm_osmium::object_order_type_id_version()(node1, way));
        REQUIRE(osrm_osmium::object_order_type_id_reverse_version()(node1, way));

        REQUIRE_FALSE(osrm_osmium::object_equal_type_id_version()(node1, node2));
        REQUIRE(osrm_osmium::object_equal_type_id_version()(node2, node3));

        REQUIRE(osrm_osmium::object_equal_type_id()(node1, node2));
        REQUIRE(osrm_osmium::object_equal_type_id()(node2, node3));

        REQUIRE_FALSE(osrm_osmium::object_equal_type_id_version()(node1, way));
        REQUIRE_FALSE(osrm_osmium::object_equal_type_id_version()(node1, relation));
        REQUIRE_FALSE(osrm_osmium::object_equal_type_id()(node1, relation));
    }

}
