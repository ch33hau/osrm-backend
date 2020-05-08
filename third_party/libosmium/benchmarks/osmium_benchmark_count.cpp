/*

  The code in this file is released into the Public Domain.

*/

#include <cstdint>
#include <iostream>

#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

struct CountHandler : public osrm_osmium::handler::Handler {

    uint64_t nodes = 0;
    uint64_t ways = 0;
    uint64_t relations = 0;

    void node(osrm_osmium::Node&) {
        ++nodes;
    }

    void way(osrm_osmium::Way&) {
        ++ways;
    }

    void relation(osrm_osmium::Relation&) {
        ++relations;
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

    std::cout << "Nodes: "     << handler.nodes << "\n";
    std::cout << "Ways: "      << handler.ways << "\n";
    std::cout << "Relations: " << handler.relations << "\n";
}

