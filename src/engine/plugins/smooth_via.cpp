#include "engine/plugins/smooth_via.hpp"

#include "engine/api/json_factory.hpp"

namespace osrm
{
namespace engine
{
namespace plugins
{

struct via_result
{
    double duration;
    double distance;
    std::vector<std::vector<Coordinate>> polylines;
};
using leg_results_t = std::vector<std::vector<std::vector<LegResult>>>;

struct ResultFinder
{

    ResultFinder(leg_results_t leg_results) : leg_results(std::move(leg_results)) {}

    via_result find_best_cached(size_t depth, size_t node_idx)
    {
        auto it = result_cache.find({depth, node_idx});
        if (it != end(result_cache))
        {
            return it->second;
        }

        auto result = find_best(depth, node_idx);
        result_cache[{depth, node_idx}] = result;
        std::cout << result.duration << std::endl;
        return result;
    }

    via_result find_best(size_t depth, size_t node_idx)
    {
        if (depth == leg_results.size() - 1)
        {
            auto min = std::min_element(begin(leg_results[depth][node_idx]),
                                        end(leg_results[depth][node_idx]),
                                        [](LegResult const &lhs, LegResult const &rhs) {
                                            return lhs.duration < rhs.duration;
                                        });
            return {min->duration, min->distance, {min->polyline}};
        }

        std::vector<via_result> tails;
        for (auto i = 0; i < leg_results[depth + 1].size(); ++i)
        {
            auto tail = find_best_cached(depth + 1, i);

            if (tail.duration == std::numeric_limits<int>::max())
            {
                continue;
            }

            if (depth != -1)
            {
                auto self = leg_results[depth][node_idx][i];

                if (self.duration == std::numeric_limits<int>::max())
                {
                    continue;
                }

                tail.duration += self.duration;
                tail.distance += self.distance;
                tail.polylines.insert(begin(tail.polylines), self.polyline);
            }

            tails.push_back(tail);
        }

        if (tails.empty())
        {
            return {std::numeric_limits<int>::max(), std::numeric_limits<double>::max(), {{}}};
        }

        return *std::min_element(
            begin(tails), end(tails), [](via_result const &lhs, via_result const &rhs) {
                return lhs.duration < rhs.duration;
            });
    }

    leg_results_t leg_results;
    std::map<std::pair<size_t, size_t>, via_result> result_cache;
};

via_result ExtractResult(leg_results_t leg_results)
{
    ResultFinder finder{std::move(leg_results)};
    return finder.find_best(-1, -1);
}

SmoothViaPlugin::SmoothViaPlugin(datafacade::BaseDataFacade &facade_)
    : BasePlugin(facade_), direct_shortest_path(&facade_, heaps), shortest_path(&facade_, heaps)
{
}

Status SmoothViaPlugin::HandleRequest(const api::SmoothViaParameters &params,
                                      util::json::Object &result)
{
    auto resolved_nodes = ResolveNodes(params);
    auto leg_results = RouteAllLegs(resolved_nodes);

    auto best_result = ExtractResult(std::move(leg_results));

    result.values["duration"] = best_result.duration;
    result.values["distance"] = best_result.distance;

    util::json::Array geometry;
    for (auto const &polyline : best_result.polylines)
    {
        geometry.values.push_back(
            api::json::makeCoordVec1DGeometry(begin(polyline), end(polyline)));
    }
    result.values["geometry"] = geometry;

    return Status::Ok;
}

std::vector<std::vector<PhantomNode>>
SmoothViaPlugin::ResolveNodes(const api::SmoothViaParameters &params)
{
    std::vector<std::vector<PhantomNode>> resolved_nodes;
    for (auto const &waypoint : params.waypoints)
    {
        // XXX can we skip tiny component nodes altogether?!?

        std::vector<PhantomNode> nodes;
        for (auto const &coord : waypoint)
        {
            std::vector<PhantomNodeWithDistance> close_nodes;
            close_nodes = facade.NearestPhantomNodesInRange(coord, 50.);
            if (close_nodes.empty() || std::all_of(begin(close_nodes),
                                                   end(close_nodes),
                                                   [](PhantomNodeWithDistance const &node) {
                                                       return node.phantom_node.component.is_tiny;
                                                   }))
            {
                auto pair = facade.NearestPhantomNodeWithAlternativeFromBigComponent(coord);
                if (!pair.first.component.is_tiny)
                {
                    nodes.push_back(pair.first);
                }
                else
                {
                    nodes.push_back(pair.second);
                }
                continue;
            }

            for (auto const &close_node : close_nodes)
            {
                nodes.push_back(close_node.phantom_node);
            }
        }

        std::sort(begin(nodes), end(nodes), [](PhantomNode const &lhs, PhantomNode const &rhs) {
            return std::tie(lhs.location.lat, lhs.location.lon) <
                   std::tie(rhs.location.lat, rhs.location.lon);
        });
        nodes.erase(std::unique(begin(nodes), end(nodes)), end(nodes));

        if (std::all_of(begin(nodes), end(nodes), [](PhantomNode const &node) {
                return node.component.is_tiny;
            }))
        {
            std::cout << "\nALL_TINY:";

            for (auto const &coord : waypoint)
            {
                std::cout << coord << " -- ";
            }
            std::cout << std::endl;
        }

        std::cout << nodes.size() << ", ";

        resolved_nodes.emplace_back(std::move(nodes));
    }
    std::cout << std::endl;

    if (resolved_nodes.size() < 2)
    {
        // TODO throw
        // throw std::system_error(error::invalid_request);
    }

    return resolved_nodes;
}

leg_results_t
SmoothViaPlugin::RouteAllLegs(const std::vector<std::vector<PhantomNode>> &resolved_nodes)
{
    leg_results_t leg_results;
    for (auto i = 1ul; i < resolved_nodes.size(); ++i)
    {
        std::vector<std::vector<LegResult>> many_to_many_results;
        for (auto const &start_node : resolved_nodes[i - 1])
        {
            std::vector<LegResult> one_to_many_results;
            for (auto const &end_node : resolved_nodes[i])
            {
                one_to_many_results.push_back(RouteDirect(start_node, end_node));
            }
            many_to_many_results.push_back(one_to_many_results);
        }
        leg_results.push_back(many_to_many_results);
    }

    return leg_results;
}

int GetTime(PhantomNode const &node, bool traversed_in_reverse)
{
    return traversed_in_reverse ? node.reverse_weight : node.forward_weight;
}

LegResult SmoothViaPlugin::RouteDirect(const PhantomNode &from, const PhantomNode &to)
{
    InternalRouteResult raw_route;
    raw_route.segment_end_coordinates.emplace_back(PhantomNodes{from, to});

    shortest_path(
        raw_route.segment_end_coordinates, {false}, raw_route); // TODO correct u-turn behavior

    if (INVALID_EDGE_WEIGHT == raw_route.shortest_path_length)
    {
        std::cout << "Error occurred, single path not found" << std::endl;

        return {std::numeric_limits<int>::max(), std::numeric_limits<double>::max(), {}};
    }

    LegResult result{0, 0.0, {}};

    if (raw_route.unpacked_path_segments.size() != 1)
    {
        std::cout << "unpacked path segments have unexpected size"
                  << raw_route.unpacked_path_segments.size() << std::endl;
    }

    result.polyline.push_back(raw_route.segment_end_coordinates[0].source_phantom.location);
    for (auto const &data : raw_route.unpacked_path_segments[0])
    {
        result.polyline.push_back(facade.GetCoordinateOfNode(data.turn_via_node));
    }
    result.polyline.push_back(raw_route.segment_end_coordinates[0].target_phantom.location);

    for (unsigned i = 1; i < result.polyline.size(); ++i)
    {
        result.distance += util::coordinate_calculation::greatCircleDistance(result.polyline[i - 1],
                                                                             result.polyline[i]);
    }

    result.duration = GetTime(raw_route.segment_end_coordinates[0].source_phantom,
                              raw_route.source_traversed_in_reverse[0]);
    for (auto const &data : raw_route.unpacked_path_segments[0])
    {
        result.duration += data.duration_until_turn;
    }
    result.duration += GetTime(raw_route.segment_end_coordinates[0].target_phantom,
                               raw_route.target_traversed_in_reverse[0]); // TODO intermediate
    // phantoms are counted
    // twice

    result.duration /= 10;

    return result;
}
}
}
}
