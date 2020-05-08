#include "catch.hpp"

#include <osmium/io/xml_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/io/writer.hpp>

TEST_CASE("output iterator") {

    SECTION("should be copy constructable") {
        osrm_osmium::io::Header header;
        osrm_osmium::io::Writer writer("test.osm", header, osrm_osmium::io::overwrite::allow);
        osrm_osmium::io::OutputIterator<osrm_osmium::io::Writer> out1(writer);

        osrm_osmium::io::OutputIterator<osrm_osmium::io::Writer> out2(out1);
    }

    SECTION("should be copy assignable") {
        osrm_osmium::io::Header header;
        osrm_osmium::io::Writer writer1("test1.osm", header, osrm_osmium::io::overwrite::allow);
        osrm_osmium::io::Writer writer2("test2.osm", header, osrm_osmium::io::overwrite::allow);

        osrm_osmium::io::OutputIterator<osrm_osmium::io::Writer> out1(writer1);
        osrm_osmium::io::OutputIterator<osrm_osmium::io::Writer> out2(writer2);

        out2 = out1;
    }

    SECTION("should be incrementable") {
        osrm_osmium::io::Header header;
        osrm_osmium::io::Writer writer("test.osm", header, osrm_osmium::io::overwrite::allow);
        osrm_osmium::io::OutputIterator<osrm_osmium::io::Writer> out(writer);

        ++out;
    }

}

