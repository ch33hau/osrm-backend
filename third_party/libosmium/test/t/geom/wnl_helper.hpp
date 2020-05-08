
#include <osmium/builder/attr.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/node_ref_list.hpp>

using namespace osrm_osrm_osmium::builder::attr;

inline const osrm_osmium::WayNodeList& create_test_wnl_okay(osrm_osmium::memory::Buffer& buffer) {
    auto pos = osrm_osmium::builder::add_way_node_list(buffer, _nodes({
        {1, {3.2, 4.2}},
        {3, {3.5, 4.7}},
        {4, {3.5, 4.7}},
        {2, {3.6, 4.9}}
    }));

    return buffer.get<osrm_osmium::WayNodeList>(pos);
}

inline const osrm_osmium::WayNodeList& create_test_wnl_empty(osrm_osmium::memory::Buffer& buffer) {
    {
        osrm_osmium::builder::WayNodeListBuilder wnl_builder(buffer);
    }

    return buffer.get<osrm_osmium::WayNodeList>(buffer.commit());
}

inline const osrm_osmium::WayNodeList& create_test_wnl_same_location(osrm_osmium::memory::Buffer& buffer) {
    auto pos = osrm_osmium::builder::add_way_node_list(buffer, _nodes({
        {1, {3.5, 4.7}},
        {2, {3.5, 4.7}}
    }));

    return buffer.get<osrm_osmium::WayNodeList>(pos);
}

inline const osrm_osmium::WayNodeList& create_test_wnl_undefined_location(osrm_osmium::memory::Buffer& buffer) {
    auto pos = osrm_osmium::builder::add_way_node_list(buffer, _nodes({
        {1, {3.5, 4.7}},
        {2, osrm_osmium::Location()}
    }));

    return buffer.get<osrm_osmium::WayNodeList>(pos);
}

