#include "catch.hpp"

#include <osmium/osm/entity_bits.hpp>

TEST_CASE("entity_bits") {

    SECTION("can_be_set_and_checked") {
        osrm_osmium::osm_entity_bits::type entities = osrm_osmium::osm_entity_bits::node | osrm_osmium::osm_entity_bits::way;
        REQUIRE(entities == (osrm_osmium::osm_entity_bits::node | osrm_osmium::osm_entity_bits::way));

        entities |= osrm_osmium::osm_entity_bits::relation;
        REQUIRE((entities & osrm_osmium::osm_entity_bits::object));

        entities |= osrm_osmium::osm_entity_bits::area;
        REQUIRE(entities == osrm_osmium::osm_entity_bits::object);

        REQUIRE(! (entities & osrm_osmium::osm_entity_bits::changeset));

        entities &= osrm_osmium::osm_entity_bits::node;
        REQUIRE((entities & osrm_osmium::osm_entity_bits::node));
        REQUIRE(! (entities & osrm_osmium::osm_entity_bits::way));
        REQUIRE(entities == osrm_osmium::osm_entity_bits::node);

        REQUIRE(osrm_osmium::osm_entity_bits::node      == osrm_osmium::osm_entity_bits::from_item_type(osrm_osmium::item_type::node));
        REQUIRE(osrm_osmium::osm_entity_bits::way       == osrm_osmium::osm_entity_bits::from_item_type(osrm_osmium::item_type::way));
        REQUIRE(osrm_osmium::osm_entity_bits::relation  == osrm_osmium::osm_entity_bits::from_item_type(osrm_osmium::item_type::relation));
        REQUIRE(osrm_osmium::osm_entity_bits::changeset == osrm_osmium::osm_entity_bits::from_item_type(osrm_osmium::item_type::changeset));
        REQUIRE(osrm_osmium::osm_entity_bits::area      == osrm_osmium::osm_entity_bits::from_item_type(osrm_osmium::item_type::area));
    }

}
