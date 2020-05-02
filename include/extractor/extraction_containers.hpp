#ifndef EXTRACTION_CONTAINERS_HPP
#define EXTRACTION_CONTAINERS_HPP

#include "extractor/external_memory_node.hpp"
#include "extractor/first_and_last_segment_of_way.hpp"
#include "extractor/guidance/turn_lane_types.hpp"
#include "extractor/internal_extractor_edge.hpp"
#include "extractor/restriction.hpp"
#include "extractor/scripting_environment.hpp"

#include <cstdint>
#include <unordered_map>

namespace osrm
{
namespace extractor
{

/**
 * Uses external memory containers from stxxl to store all the data that
 * is collected by the extractor callbacks.
 *
 * The data is the filtered, aggregated and finally written to disk.
 */
class ExtractionContainers
{
    void PrepareNodes();
    void PrepareRestrictions();
    void PrepareEdges(lua_State *segment_state);

    void WriteNodes(std::ofstream &file_out_stream) const;
    void WriteRestrictions(const std::string &restrictions_file_name) const;
    void WriteEdges(std::ofstream &file_out_stream) const;
    void WriteCharData(const std::string &file_name);
    void
    WriteTurnLaneMasks(const std::string &file_name,
                       const std::vector<std::uint32_t> &turn_lane_offsets,
                       const std::vector<guidance::TurnLaneType::Mask> &turn_lane_masks) const;

  public:
    using STXXLNodeIDVector = std::vector<OSMNodeID>;
    using STXXLNodeVector = std::vector<ExternalMemoryNode>;
    using STXXLEdgeVector = std::vector<InternalExtractorEdge>;
    using STXXLRestrictionsVector = std::vector<InputRestrictionContainer>;
    using STXXLWayIDStartEndVector = std::vector<FirstAndLastSegmentOfWay>;
    using STXXLNameCharData = std::vector<unsigned char>;
    using STXXLNameOffsets = std::vector<unsigned>;

    STXXLNodeIDVector used_node_id_list;
    STXXLNodeVector all_nodes_list;
    STXXLEdgeVector all_edges_list;
    STXXLNameCharData name_char_data;
    STXXLNameOffsets name_offsets;
    // an adjacency array containing all turn lane masks
    std::vector<std::uint32_t> turn_lane_offsets;
    std::vector<guidance::TurnLaneType::Mask> turn_lane_masks;
    STXXLRestrictionsVector restrictions_list;
    STXXLWayIDStartEndVector way_start_end_id_list;
    std::unordered_map<OSMNodeID, NodeID> external_to_internal_node_id_map;
    unsigned max_internal_node_id;

    ExtractionContainers();

    void PrepareData(const std::string &output_file_name,
                     const std::string &restrictions_file_name,
                     const std::string &names_file_name,
                     const std::string &turn_lane_file_name,
                     lua_State *segment_state);
};
}
}

#endif /* EXTRACTION_CONTAINERS_HPP */
