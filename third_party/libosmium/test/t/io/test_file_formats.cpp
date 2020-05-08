#include "catch.hpp"

#include <iterator>

#include <osmium/io/file.hpp>

TEST_CASE("FileFormats") {

    SECTION("default_file_format") {
        osrm_osmium::io::File f;
        REQUIRE(osrm_osmium::io::file_format::unknown == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("stdin_stdout_empty") {
        osrm_osmium::io::File f {""};
        REQUIRE(osrm_osmium::io::file_format::unknown == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("stdin_stdout_dash") {
        osrm_osmium::io::File f {"-"};
        REQUIRE(osrm_osmium::io::file_format::unknown == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("stdin_stdout_bz2") {
        osrm_osmium::io::File f {"-", "osm.bz2"};
        REQUIRE("" == f.filename());
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::bzip2 == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osm") {
        osrm_osmium::io::File f {"test.osm"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_pbf") {
        osrm_osmium::io::File f {"test.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osm_pbf") {
        osrm_osmium::io::File f {"test.osm.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_opl") {
        osrm_osmium::io::File f {"test.opl"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osm_opl") {
        osrm_osmium::io::File f {"test.osm.opl"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osm_gz") {
        osrm_osmium::io::File f {"test.osm.gz"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_opl_bz2") {
        osrm_osmium::io::File f {"test.osm.opl.bz2"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::bzip2 == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osc_gz") {
        osrm_osmium::io::File f {"test.osc.gz"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_opl_gz") {
        osrm_osmium::io::File f {"test.osh.opl.gz"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("detect_file_format_by_suffix_osh_pbf") {
        osrm_osmium::io::File f {"test.osh.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osm") {
        osrm_osmium::io::File f {"test", "osm"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_pbf") {
        osrm_osmium::io::File f {"test", "pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osm_pbf") {
        osrm_osmium::io::File f {"test", "osm.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_opl") {
        osrm_osmium::io::File f {"test", "opl"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osm_opl") {
        osrm_osmium::io::File f {"test", "osm.opl"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osm_gz") {
        osrm_osmium::io::File f {"test", "osm.gz"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osm_opl_bz2") {
        osrm_osmium::io::File f {"test", "osm.opl.bz2"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::bzip2 == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osc_gz") {
        osrm_osmium::io::File f {"test", "osc.gz"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osh_opl_gz") {
        osrm_osmium::io::File f {"test", "osh.opl.gz"};
        REQUIRE(osrm_osmium::io::file_format::opl == f.format());
        REQUIRE(osrm_osmium::io::file_compression::gzip == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("override_file_format_by_suffix_osh_pbf") {
        osrm_osmium::io::File f {"test", "osh.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("format_options_pbf_history") {
        osrm_osmium::io::File f {"test", "pbf,history=true"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("format_options_pbf_foo") {
        osrm_osmium::io::File f {"test.osm", "pbf,foo=bar"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE("bar" == f.get("foo"));
        f.check();
    }

    SECTION("format_options_xml_abc_something") {
        osrm_osmium::io::File f {"test.bla", "xml,abc,some=thing"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE("true" == f.get("abc"));
        REQUIRE("thing" == f.get("some"));
        REQUIRE(2 == std::distance(f.begin(), f.end()));
        f.check();
    }

    SECTION("unknown_format_foo_bar") {
        osrm_osmium::io::File f {"test.foo.bar"};
        REQUIRE(osrm_osmium::io::file_format::unknown == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("unknown_format_foo") {
        osrm_osmium::io::File f {"test", "foo"};
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("unknown_format_osm_foo") {
        osrm_osmium::io::File f {"test", "osm.foo"};
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("unknown_format_bla_equals_foo") {
        osrm_osmium::io::File f {"test", "bla=foo"};
        REQUIRE_THROWS_AS(f.check(), std::runtime_error);
    }

    SECTION("url without format") {
        osrm_osmium::io::File f {"http://www.example.com/api"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("url without format and filename") {
        osrm_osmium::io::File f {"http://planet.osm.org/pbf/planet-latest.osm.pbf"};
        REQUIRE(osrm_osmium::io::file_format::pbf == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(false == f.has_multiple_object_versions());
        f.check();
    }

    SECTION("url with format") {
        osrm_osmium::io::File f {"http://www.example.com/api", "osh"};
        REQUIRE(osrm_osmium::io::file_format::xml == f.format());
        REQUIRE(osrm_osmium::io::file_compression::none == f.compression());
        REQUIRE(true == f.has_multiple_object_versions());
        f.check();
    }

}

