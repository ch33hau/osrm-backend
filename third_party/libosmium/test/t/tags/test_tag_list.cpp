#include "catch.hpp"

#include <map>
#include <vector>

#include <osmium/builder/attr.hpp>
#include <osmium/builder/builder_helper.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/tag.hpp>

using namespace osrm_osrm_osmium::builder::attr;

TEST_CASE("create tag list") {
    osrm_osmium::memory::Buffer buffer(10240);

    SECTION("with TagListBuilder from char*") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag("highway", "primary");
            builder.add_tag("name", "Main Street");
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from pair<const char*, const char*>") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag(std::pair<const char*, const char*>{"highway", "primary"});
            builder.add_tag("name", "Main Street");
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from pair<const char* const, const char*>") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag(std::pair<const char* const, const char*>{"highway", "primary"});
            builder.add_tag("name", "Main Street");
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from pair<const char*, const char* const>") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag(std::pair<const char*, const char* const>{"highway", "primary"});
            builder.add_tag("name", "Main Street");
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from pair<const char* const, const char* const>") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag(std::pair<const char* const, const char* const>{"highway", "primary"});
            builder.add_tag("name", "Main Street");
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from char* with length") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag("highway", strlen("highway"), "primary", strlen("primary"));
            builder.add_tag("nameXX", 4, "Main Street", 11);
        }
        buffer.commit();
    }

    SECTION("with TagListBuilder from std::string") {
        {
            osrm_osmium::builder::TagListBuilder builder(buffer);
            builder.add_tag(std::string("highway"), std::string("primary"));
            const std::string source = "name";
            std::string gps = "Main Street";
            builder.add_tag(source, gps);
        }
        buffer.commit();
    }

    SECTION("with add_tag_list from pair<const char*, const char*>") {
        osrm_osmium::builder::add_tag_list(buffer,
            _tag(std::pair<const char*, const char*>{"highway", "primary"}),
            _tag("name", "Main Street")
        );
    }

    SECTION("with add_tag_list from pair<const char* const, const char*>") {
        osrm_osmium::builder::add_tag_list(buffer,
            _tag(std::pair<const char* const, const char*>{"highway", "primary"}),
            _tag("name", "Main Street")
        );
    }

    SECTION("with add_tag_list from pair<const char*, const char* const>") {
        osrm_osmium::builder::add_tag_list(buffer,
            _tag(std::pair<const char*, const char* const>{"highway", "primary"}),
            _tag("name", "Main Street")
        );
    }

    SECTION("with add_tag_list from pair<const char* const, const char* const>") {
        osrm_osmium::builder::add_tag_list(buffer,
            _tag(std::pair<const char* const, const char* const>{"highway", "primary"}),
            _tag("name", "Main Street")
        );
    }

    SECTION("with add_tag_list from vector of pairs (const/const)") {
        std::vector<std::pair<const char* const, const char* const>> v{
            { "highway", "primary" },
            { "name", "Main Street" }
        };
        osrm_osmium::builder::add_tag_list(buffer, _tags(v));
    }

    SECTION("with add_tag_list from vector of pairs (const/nc)") {
        std::vector<std::pair<const char* const, const char*>> v{
            { "highway", "primary" },
            { "name", "Main Street" }
        };
        osrm_osmium::builder::add_tag_list(buffer, _tags(v));
    }

    SECTION("with add_tag_list from vector of pairs (nc/const)") {
        std::vector<std::pair<const char*, const char* const>> v{
            { "highway", "primary" },
            { "name", "Main Street" }
        };
        osrm_osmium::builder::add_tag_list(buffer, _tags(v));
    }

    SECTION("with add_tag_list from vector of pairs (nc/nc)") {
        std::vector<std::pair<const char*, const char*>> v{
            { "highway", "primary" },
            { "name", "Main Street" }
        };
        osrm_osmium::builder::add_tag_list(buffer, _tags(v));
    }

    SECTION("with add_tag_list from initializer list") {
        osrm_osmium::builder::add_tag_list(buffer, _tags({
            { "highway", "primary" },
            { "name", "Main Street" }
        }));
    }

    SECTION("with add_tag_list from _tag") {
        osrm_osmium::builder::add_tag_list(buffer,
            _tag("highway", "primary"),
            _tag("name", "Main Street")
        );
    }

    SECTION("with add_tag_list from map") {
        std::map<const char*, const char*> m{
            { "highway", "primary" },
            { "name", "Main Street" }
        };
        osrm_osmium::builder::add_tag_list(buffer, _tags(m));
    }

    SECTION("with build_tag_list_from_func") {
        osrm_osmium::builder::build_tag_list_from_func(buffer, [](osrm_osmium::builder::TagListBuilder& tlb) {
            tlb.add_tag("highway", "primary");
            tlb.add_tag("name", "Main Street");
        });
    }

    const osrm_osmium::TagList& tl = *buffer.begin<osrm_osmium::TagList>();
    REQUIRE(osrm_osmium::item_type::tag_list == tl.type());
    REQUIRE(2 == tl.size());

    auto it = tl.begin();
    REQUIRE(std::string("highway")     == it->key());
    REQUIRE(std::string("primary")     == it->value());
    ++it;
    REQUIRE(std::string("name")        == it->key());
    REQUIRE(std::string("Main Street") == it->value());
    ++it;
    REQUIRE(it == tl.end());

    REQUIRE(std::string("primary") == tl.get_value_by_key("highway"));
    REQUIRE(nullptr == tl.get_value_by_key("foo"));
    REQUIRE(std::string("default") == tl.get_value_by_key("foo", "default"));

    REQUIRE(std::string("Main Street") == tl["name"]);
}

TEST_CASE("empty keys and values are okay") {
    osrm_osmium::memory::Buffer buffer(10240);

    auto pos = osrm_osmium::builder::add_tag_list(buffer,
        _tag("empty value", ""),
        _tag("", "empty key")
    );
    const osrm_osmium::TagList& tl = buffer.get<osrm_osmium::TagList>(pos);

    REQUIRE(osrm_osmium::item_type::tag_list == tl.type());
    REQUIRE(2 == tl.size());

    auto it = tl.begin();
    REQUIRE(std::string("empty value") == it->key());
    REQUIRE(std::string("")            == it->value());
    ++it;
    REQUIRE(std::string("")            == it->key());
    REQUIRE(std::string("empty key")   == it->value());
    ++it;
    REQUIRE(it == tl.end());

    REQUIRE(std::string("") == tl.get_value_by_key("empty value"));
    REQUIRE(std::string("empty key") == tl.get_value_by_key(""));
}

TEST_CASE("tag key or value is too long") {
    osrm_osmium::memory::Buffer buffer(10240);
    osrm_osmium::builder::TagListBuilder builder(buffer);

    const char kv[2000] = "";
    builder.add_tag(kv, 1, kv, 1000);
    REQUIRE_THROWS(builder.add_tag(kv, 1500, kv, 1));
    REQUIRE_THROWS(builder.add_tag(kv, 1, kv, 1500));
}

