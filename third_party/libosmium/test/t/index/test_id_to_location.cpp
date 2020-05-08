#include "catch.hpp"

#include <osmium/osm/types.hpp>
#include <osmium/osm/location.hpp>

#include <osmium/index/map/dense_file_array.hpp>
#include <osmium/index/map/dense_mem_array.hpp>
#include <osmium/index/map/dense_mmap_array.hpp>
#include <osmium/index/map/dummy.hpp>
#include <osmium/index/map/sparse_file_array.hpp>
#include <osmium/index/map/sparse_mem_array.hpp>
#include <osmium/index/map/sparse_mem_map.hpp>
#include <osmium/index/map/sparse_mem_table.hpp>
#include <osmium/index/map/sparse_mmap_array.hpp>

#include <osmium/index/node_locations_map.hpp>

template <typename TIndex>
void test_func_all(TIndex& index) {
    osrm_osmium::unsigned_object_id_type id1 = 12;
    osrm_osmium::unsigned_object_id_type id2 = 3;
    osrm_osmium::Location loc1(1.2, 4.5);
    osrm_osmium::Location loc2(3.5, -7.2);

    REQUIRE_THROWS_AS(index.get(id1), osrm_osmium::not_found);

    index.set(id1, loc1);
    index.set(id2, loc2);

    index.sort();

    REQUIRE_THROWS_AS(index.get(5), osrm_osmium::not_found);
    REQUIRE_THROWS_AS(index.get(100), osrm_osmium::not_found);
}

template <typename TIndex>
void test_func_real(TIndex& index) {
    osrm_osmium::unsigned_object_id_type id1 = 12;
    osrm_osmium::unsigned_object_id_type id2 = 3;
    osrm_osmium::Location loc1(1.2, 4.5);
    osrm_osmium::Location loc2(3.5, -7.2);

    index.set(id1, loc1);
    index.set(id2, loc2);

    index.sort();

    REQUIRE(loc1 == index.get(id1));
    REQUIRE(loc2 == index.get(id2));

    REQUIRE_THROWS_AS(index.get(5), osrm_osmium::not_found);
    REQUIRE_THROWS_AS(index.get(100), osrm_osmium::not_found);

    index.clear();

    REQUIRE_THROWS_AS(index.get(id1), osrm_osmium::not_found);
}

TEST_CASE("IdToLocation") {

    SECTION("Dummy") {
        typedef osrm_osmium::index::map::Dummy<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;

        REQUIRE(0 == index1.size());
        REQUIRE(0 == index1.used_memory());

        test_func_all<index_type>(index1);

        REQUIRE(0 == index1.size());
        REQUIRE(0 == index1.used_memory());
    }

    SECTION("DenseMemArray") {
        typedef osrm_osmium::index::map::DenseMemArray<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;
        index1.reserve(1000);
        test_func_all<index_type>(index1);

        index_type index2;
        index2.reserve(1000);
        test_func_real<index_type>(index2);
    }

#ifdef __linux__
    SECTION("DenseMmapArray") {
        typedef osrm_osmium::index::map::DenseMmapArray<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;
        test_func_all<index_type>(index1);

        index_type index2;
        test_func_real<index_type>(index2);
    }
#else
# pragma message("not running 'DenseMapMmap' test case on this machine")
#endif

    SECTION("DenseFileArray") {
        typedef osrm_osmium::index::map::DenseFileArray<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;
        test_func_all<index_type>(index1);

        index_type index2;
        test_func_real<index_type>(index2);
    }

#ifdef OSMIUM_WITH_SPARSEHASH

    SECTION("SparseMemTable") {
        typedef osrm_osmium::index::map::SparseMemTable<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;
        test_func_all<index_type>(index1);

        index_type index2;
        test_func_real<index_type>(index2);
    }

#endif

    SECTION("SparseMemMap") {
        typedef osrm_osmium::index::map::SparseMemMap<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;
        test_func_all<index_type>(index1);

        index_type index2;
        test_func_real<index_type>(index2);
    }

    SECTION("SparseMemArray") {
        typedef osrm_osmium::index::map::SparseMemArray<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> index_type;

        index_type index1;

        REQUIRE(0 == index1.size());
        REQUIRE(0 == index1.used_memory());

        test_func_all<index_type>(index1);

        REQUIRE(2 == index1.size());

        index_type index2;
        test_func_real<index_type>(index2);
    }

    SECTION("Dynamic map choice") {
        typedef osrm_osmium::index::map::Map<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location> map_type;
        const auto& map_factory = osrm_osmium::index::MapFactory<osrm_osmium::unsigned_object_id_type, osrm_osmium::Location>::instance();

        std::vector<std::string> map_type_names = map_factory.map_types();
        REQUIRE(map_type_names.size() >= 5);

        for (const auto& map_type_name : map_type_names) {
            std::unique_ptr<map_type> index1 = map_factory.create_map(map_type_name);
            index1->reserve(1000);
            test_func_all<map_type>(*index1);

            std::unique_ptr<map_type> index2 = map_factory.create_map(map_type_name);
            index2->reserve(1000);
            test_func_real<map_type>(*index2);
        }
    }

}

