/*
Copyright (c) 2014, Project OSRM, Felix Guendling
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MULTI_TARGET_ROUTING_H
#define MULTI_TARGET_ROUTING_H

#include "util/typedefs.hpp"

#include "engine/routing_algorithms/routing_base.hpp"

#include "engine/search_engine_data.hpp"

#include <boost/assert.hpp>

#include <memory>
#include <vector>

namespace osrm
{
namespace engine
{
namespace routing_algorithms
{

template <class DataFacadeT, bool forward>
class MultiTargetRouting final
    : public BasicRoutingInterface<DataFacadeT, MultiTargetRouting<DataFacadeT, forward>>
{
    typedef BasicRoutingInterface<DataFacadeT, MultiTargetRouting<DataFacadeT, forward>> super;
    typedef SearchEngineData::QueryHeap QueryHeap;
    SearchEngineData &engine_working_data;

  public:
    MultiTargetRouting(DataFacadeT *facade, SearchEngineData &engine_working_data)
        : super(facade), engine_working_data(engine_working_data)
    {
    }

    ~MultiTargetRouting() {}

    std::shared_ptr<std::vector<std::pair<double, double>>>
    operator()(const std::vector<PhantomNode> &phantom_nodes_array) const
    {
        BOOST_ASSERT(phantom_nodes_array.size() >= 2);

        // Prepare results table:
        // Each target (phantom_nodes_array[1], ..., phantom_nodes_array[n]) has its own index.
        auto results = std::make_shared<std::vector<std::pair<double, double>>>();
        results->reserve(phantom_nodes_array.size() - 1);

        // For readability: some reference variables making it clear,
        // which one is the source and which are the targets.
        const auto &source = phantom_nodes_array[0];
        std::vector<std::reference_wrapper<const PhantomNode>> targets(
            std::next(begin(phantom_nodes_array)), end(phantom_nodes_array));

        engine_working_data.InitializeOrClearFirstThreadLocalStorage(
            super::facade->GetNumberOfNodes());

        // The forward heap keeps the distances from the source.
        // Therefore it will be reused for each target backward search.
        QueryHeap &forward_heap = *(engine_working_data.forward_heap_1);

        // The reverse heap will be cleared after each search from
        // one of the targets to the source.
        QueryHeap &reverse_heap = *(engine_working_data.reverse_heap_1);

        // Fill forward heap with the source location phantom node(s).
        // The source location is located at index 0.
        // The target locations are located at index [1, ..., n].
        EdgeWeight min_edge_offset = 0;
        if (source.forward_segment_id.enabled)
        {
            EdgeWeight offset = (forward ? -1 : 1) * source.GetForwardWeightPlusOffset();
            forward_heap.Insert(source.forward_segment_id.id, offset,
                                source.forward_segment_id.id);
            min_edge_offset = std::min(min_edge_offset, offset);
        }
        if (source.reverse_segment_id.enabled)
        {
            EdgeWeight offset = (forward ? -1 : 1) * source.GetReverseWeightPlusOffset();
            forward_heap.Insert(source.reverse_segment_id.id,
                                (forward ? -1 : 1) * source.GetReverseWeightPlusOffset(),
                                source.reverse_segment_id.id);
            min_edge_offset = std::min(min_edge_offset, offset);
        }

        for (auto const &target : targets)
        {
            auto result = FindShortestPath(source, target, forward_heap, reverse_heap,
                                           min_edge_offset);
            results->emplace_back(std::move(result));
        }

        forward_heap.Clear();
        reverse_heap.Clear();

        return results;
    }

    std::pair<double, double> FindShortestPath(const PhantomNode &source,
                                               const PhantomNode &target,
                                               QueryHeap &forward_heap,
                                               QueryHeap &backward_heap,
                                               EdgeWeight min_edge_offset) const
    {
        NodeID middle = UINT_MAX;
        int local_upper_bound = INT_MAX;

        // Clear backward heap from the entries produced by the search to the last target
        // and initialize heap for this target.
        backward_heap.Clear();
        if (target.forward_segment_id.enabled)
        {
            EdgeWeight offset = (forward ? 1 : -1) * target.GetForwardWeightPlusOffset();
            backward_heap.Insert(target.forward_segment_id.id, offset,
                                 target.forward_segment_id.id);
            min_edge_offset = std::min(min_edge_offset, offset);
        }

        if (target.reverse_segment_id.enabled)
        {
            EdgeWeight offset = (forward ? 1 : -1) * target.GetReverseWeightPlusOffset();
            backward_heap.Insert(target.reverse_segment_id.id, offset,
                                 target.reverse_segment_id.id);
            min_edge_offset = std::min(min_edge_offset, offset);
        }

        // Execute bidirectional Dijkstra shortest path search.
        while (0 < backward_heap.Size() || 0 < forward_heap.Size())
        {
            if (0 < forward_heap.Size())
            {
                super::RoutingStep(forward_heap, backward_heap, middle, local_upper_bound,
                                   min_edge_offset, forward, false, false, false, false);
            }
            if (0 < backward_heap.Size())
            {
                super::RoutingStep(backward_heap, forward_heap, middle, local_upper_bound,
                                   min_edge_offset, !forward, false, false, false);
            }
        }

        // Check if no path could be found (-> early exit).
        if (INVALID_EDGE_WEIGHT == local_upper_bound)
        {
            return std::make_pair(INVALID_EDGE_WEIGHT, 0);
        }

        // Calculate exact distance in km.
        std::vector<NodeID> packed_path;
        super::RetrievePackedPathFromHeap(forward_heap, backward_heap, middle, packed_path);

        if (!forward)
        {
            std::reverse(begin(packed_path), end(packed_path));
        }

        std::vector<PathData> unpacked_path;
        if (forward)
        {
            super::UnpackPath(begin(packed_path), end(packed_path), 
                              {source, target}, unpacked_path);
        }
        else
        {
            super::UnpackPath(begin(packed_path), end(packed_path), 
                              {target, source}, unpacked_path);
        }

        std::vector<Coordinate> coordinates;
        coordinates.reserve(unpacked_path.size() + 2);

        coordinates.emplace_back(forward ? source.location
                                         : target.location);
        for (const auto &path_data : unpacked_path)
        {
            coordinates.emplace_back(super::facade->GetCoordinateOfNode(path_data.turn_via_node));
        }
        coordinates.emplace_back(forward ? target.location
                                         : source.location);

        double distance = 0.0;
        for (unsigned i = 1; i < coordinates.size(); ++i)
        {
            distance +=
                util::coordinate_calculation::greatCircleDistance(coordinates[i - 1], coordinates[i]);
        }

        return std::make_pair(static_cast<double>(local_upper_bound) / 10., distance);
    }
};
}
}
}


#endif
