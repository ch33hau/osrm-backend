#include "catch.hpp"
#include "utils.hpp"

#include <algorithm>

#include <osmium/io/any_compression.hpp>
#include <osmium/io/xml_input.hpp>
#include <osmium/io/xml_output.hpp>
#include <osmium/io/output_iterator.hpp>
#include <osmium/memory/buffer.hpp>

TEST_CASE("Writer") {

    osrm_osmium::io::Header header;
    header.set("generator", "test_writer.cpp");

    osrm_osmium::io::Reader reader(with_data_dir("t/io/data.osm"));
    osrm_osmium::memory::Buffer buffer = reader.read();
    REQUIRE(buffer);
    REQUIRE(buffer.committed() > 0);
    auto num = std::distance(buffer.cbegin<osrm_osmium::OSMObject>(), buffer.cend<osrm_osmium::OSMObject>());
    REQUIRE(num > 0);
    REQUIRE(buffer.cbegin<osrm_osmium::OSMObject>()->id() == 1);

    std::string filename;

    SECTION("Empty writes") {

        SECTION("Empty buffer") {
            filename = "test-writer-out-empty-buffer.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            osrm_osmium::memory::Buffer empty_buffer(1024);
            writer(std::move(empty_buffer));
            writer.close();
        }

        SECTION("Invalid buffer") {
            filename = "test-writer-out-invalid-buffer.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            osrm_osmium::memory::Buffer invalid_buffer;
            writer(std::move(invalid_buffer));
            writer.close();
        }

        osrm_osmium::io::Reader reader_check(filename);
        osrm_osmium::memory::Buffer buffer_check = reader_check.read();
        REQUIRE(!buffer_check);
    }

    SECTION("Successfull writes") {

        SECTION("Writer buffer") {
            filename = "test-writer-out-buffer.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            writer(std::move(buffer));
            writer.close();

            REQUIRE_THROWS_AS({
                writer(osrm_osmium::memory::Buffer{});
            }, osrm_osmium::io_error);
        }

        SECTION("Writer item") {
            filename = "test-writer-out-item.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            for (const auto& item : buffer) {
                writer(item);
            }
            writer.close();
        }

        SECTION("Writer output iterator") {
            filename = "test-writer-out-iterator.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            auto it = osrm_osmium::io::make_output_iterator(writer);
            std::copy(buffer.cbegin(), buffer.cend(), it);
            writer.close();
        }

        osrm_osmium::io::Reader reader_check(filename);
        osrm_osmium::memory::Buffer buffer_check = reader_check.read();
        REQUIRE(buffer_check);
        REQUIRE(buffer_check.committed() > 0);
        REQUIRE(std::distance(buffer_check.cbegin<osrm_osmium::OSMObject>(), buffer_check.cend<osrm_osmium::OSMObject>()) == num);
        REQUIRE(buffer_check.cbegin<osrm_osmium::OSMObject>()->id() == 1);

    }

    SECTION("Interrupted writer after open") {
        int error = 0;
        try {
            filename = "test-writer-out-fail1.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            throw 1;
        } catch (int e) {
            error = e;
        }

        REQUIRE(error > 0);
    }

    SECTION("Interrupted writer after write") {
        int error = 0;
        try {
            filename = "test-writer-out-fail2.osm";
            osrm_osmium::io::Writer writer(filename, header, osrm_osmium::io::overwrite::allow);
            writer(std::move(buffer));
            throw 2;
        } catch (int e) {
            error = e;
        }

        REQUIRE(error > 0);
    }

}


