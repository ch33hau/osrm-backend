/*

  The code in this file is released into the Public Domain.

*/

#include <iostream>

#include <osmium/index/map/all.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/visitor.hpp>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>

typedef osrm_osmium::index::map::Map<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

typedef osrm_osmium::handler::NodeLocationsForWays<index_type> location_handler_type;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE FORMAT\n";
        exit(1);
    }

    std::string input_filename = argv[1];
    std::string location_store = argv[2];

    osrm_osmium::io::Reader reader(input_filename);

    const auto& map_factory = osrm_osmium::index::MapFactory<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location>::instance();
    std::unique_ptr<index_type> index = map_factory.create_map(location_store);
    location_handler_type location_handler(*index);
    location_handler.ignore_errors();

    osrm_osmium::apply(reader, location_handler);
    reader.close();
}

