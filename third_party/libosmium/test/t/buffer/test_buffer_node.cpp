#include "catch.hpp"

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/osm/node.hpp>

void check_node_1(osrm_osmium::Node& node) {
    REQUIRE(1 == node.id());
    REQUIRE(3 == node.version());
    REQUIRE(true == node.visible());
    REQUIRE(333 == node.changeset());
    REQUIRE(21 == node.uid());
    REQUIRE(123 == uint32_t(node.timestamp()));
    REQUIRE(osrm_osmium::Location(3.5, 4.7) == node.location());
    REQUIRE(std::string("testuser") == node.user());

    for (osrm_osmium::memory::Item& item : node) {
        REQUIRE(osrm_osmium::item_type::tag_list == item.type());
    }

    REQUIRE(node.tags().begin() == node.tags().end());
    REQUIRE(node.tags().empty());
    REQUIRE(0 == std::distance(node.tags().begin(), node.tags().end()));
}

void check_node_2(osrm_osmium::Node& node) {
    REQUIRE(2 == node.id());
    REQUIRE(3 == node.version());
    REQUIRE(true == node.visible());
    REQUIRE(333 == node.changeset());
    REQUIRE(21 == node.uid());
    REQUIRE(123 == uint32_t(node.timestamp()));
    REQUIRE(osrm_osmium::Location(3.5, 4.7) == node.location());
    REQUIRE(std::string("testuser") == node.user());

    for (osrm_osmium::memory::Item& item : node) {
        REQUIRE(osrm_osmium::item_type::tag_list == item.type());
    }

    REQUIRE(!node.tags().empty());
    REQUIRE(2 == std::distance(node.tags().begin(), node.tags().end()));

    int n = 0;
    for (const osrm_osmium::Tag& tag : node.tags()) {
        switch (n) {
            case 0:
                REQUIRE(std::string("amenity") == tag.key());
                REQUIRE(std::string("bank") == tag.value());
                break;
            case 1:
                REQUIRE(std::string("name") == tag.key());
                REQUIRE(std::string("OSM Savings") == tag.value());
                break;
        }
        ++n;
    }
    REQUIRE(2 == n);
}

TEST_CASE("Node in Buffer") {

    constexpr size_t buffer_size = 10000;
    unsigned char data[buffer_size];

    osrm_osmium::memory::Buffer buffer(data, buffer_size, 0);

    SECTION("Add node to buffer") {

        {
            // add node 1
            osrm_osmium::builder::NodeBuilder node_builder(buffer);
            osrm_osmium::Node& node = node_builder.object();
            REQUIRE(osrm_osmium::item_type::node == node.type());

            node.set_id(1);
            node.set_version(3);
            node.set_visible(true);
            node.set_changeset(333);
            node.set_uid(21);
            node.set_timestamp(123);
            node.set_location(osrm_osmium::Location(3.5, 4.7));

            node_builder.add_user("testuser");

            buffer.commit();
        }

        {
            // add node 2
            osrm_osmium::builder::NodeBuilder node_builder(buffer);
            osrm_osmium::Node& node = node_builder.object();
            REQUIRE(osrm_osmium::item_type::node == node.type());

            node.set_id(2);
            node.set_version(3);
            node.set_visible(true);
            node.set_changeset(333);
            node.set_uid(21);
            node.set_timestamp(123);
            node.set_location(osrm_osmium::Location(3.5, 4.7));

            node_builder.add_user("testuser");

            {
                osrm_osmium::builder::TagListBuilder tag_builder(buffer, &node_builder);
                tag_builder.add_tag("amenity", "bank");
                tag_builder.add_tag("name", "OSM Savings");
            }

            buffer.commit();
        }

        REQUIRE(2 == std::distance(buffer.begin(), buffer.end()));
        int item_no = 0;
        for (osrm_osmium::memory::Item& item : buffer) {
            REQUIRE(osrm_osmium::item_type::node == item.type());

            osrm_osmium::Node& node = static_cast<osrm_osmium::Node&>(item);

            switch (item_no) {
                case 0:
                    check_node_1(node);
                    break;
                case 1:
                    check_node_2(node);
                    break;
                default:
                    break;
            }

            ++item_no;

        }

    }

    SECTION("Add buffer to another one") {

        {
            // add node 1
            osrm_osmium::builder::NodeBuilder node_builder(buffer);
            osrm_osmium::Node& node = node_builder.object();
            REQUIRE(osrm_osmium::item_type::node == node.type());

            node.set_id(1);
            node.set_version(3);
            node.set_visible(true);
            node.set_changeset(333);
            node.set_uid(21);
            node.set_timestamp(123);
            node.set_location(osrm_osmium::Location(3.5, 4.7));

            node_builder.add_user("testuser");

            buffer.commit();
        }

        osrm_osmium::memory::Buffer buffer2(buffer_size, osrm_osmium::memory::Buffer::auto_grow::yes);

        buffer2.add_buffer(buffer);
        buffer2.commit();

        REQUIRE(buffer.committed() == buffer2.committed());
        const osrm_osmium::Node& node = buffer2.get<osrm_osmium::Node>(0);
        REQUIRE(node.id() == 1);
        REQUIRE(123 == uint32_t(node.timestamp()));
    }

    SECTION("Use back_inserter on buffer") {

        {
            // add node 1
            osrm_osmium::builder::NodeBuilder node_builder(buffer);
            osrm_osmium::Node& node = node_builder.object();
            REQUIRE(osrm_osmium::item_type::node == node.type());

            node.set_id(1);
            node.set_version(3);
            node.set_visible(true);
            node.set_changeset(333);
            node.set_uid(21);
            node.set_timestamp(123);
            node.set_location(osrm_osmium::Location(3.5, 4.7));

            node_builder.add_user("testuser");

            buffer.commit();
        }

        osrm_osmium::memory::Buffer buffer2(buffer_size, osrm_osmium::memory::Buffer::auto_grow::yes);

        std::copy(buffer.begin(), buffer.end(), std::back_inserter(buffer2));

        REQUIRE(buffer.committed() == buffer2.committed());
        const osrm_osmium::Node& node = buffer2.get<osrm_osmium::Node>(0);
        REQUIRE(node.id() == 1);
        REQUIRE(123 == uint32_t(node.timestamp()));
    }
}

