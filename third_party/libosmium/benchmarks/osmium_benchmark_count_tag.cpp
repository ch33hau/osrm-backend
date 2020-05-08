/*

  The code in this file is released into the Public Domain.

*/

#include <cstdint>
#include <iostream>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

struct CountHandler : public osrm_osmium::handler::Handler {

    uint64_t counter = 0;
    uint64_t all = 0;

    void node(osrm_osmium::Node& node) {
        ++all;
        const char* amenity = node.tags().get_value_by_key("amenity");
        if (amenity && !strcmp(amenity, "post_box")) {
            ++counter;
        }
    }

    void way(osrm_osmium::Way&) {
        ++all;
    }

    void relation(osrm_osmium::Relation&) {
        ++all;
    }

};


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " OSMFILE\n";
        exit(1);
    }

    std::string input_filename = argv[1];

    osrm_osmium::io::Reader reader(input_filename);

    CountHandler handler;
    osrm_osmium::apply(reader, handler);
    reader.close();

    std::cout << "r_all=" << handler.all << " r_counter="  << handler.counter << "\n";
}

