#ifndef BASE_PLUGIN_HPP
#define BASE_PLUGIN_HPP

#include "engine/api/base_parameters.hpp"
#include "engine/datafacade/datafacade_base.hpp"
#include "engine/phantom_node.hpp"
#include "engine/status.hpp"

#include "util/coordinate.hpp"
#include "util/coordinate_calculation.hpp"
#include "util/integer_range.hpp"
#include "util/json_container.hpp"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace osrm
{
namespace engine
{
namespace plugins
{

class BasePlugin
{
  protected:
    datafacade::BaseDataFacade &facade;
    BasePlugin(datafacade::BaseDataFacade &facade_) : facade(facade_) {}

    bool CheckAllCoordinates(const std::vector<util::Coordinate> &coordinates)
    {
        return !std::any_of(
            std::begin(coordinates), std::end(coordinates), [](const util::Coordinate coordinate) {
                return !coordinate.IsValid();
            });
    }

    Status Error(const std::string &code,
                 const std::string &message,
                 util::json::Object &json_result) const
    {
        json_result.values["code"] = code;
        json_result.values["message"] = message;
        return Status::Error;
    }

    // Decides whether to use the phantom node from a big or small component if both are found.
    // Returns true if all phantom nodes are in the same component after snapping.
    std::vector<PhantomNode>
    SnapPhantomNodes(const std::vector<PhantomNodePair> &phantom_node_pair_list) const
    {
        const auto check_component_id_is_tiny =
            [](const std::pair<PhantomNode, PhantomNode> &phantom_pair) {
                return phantom_pair.first.component.is_tiny;
            };

        // are all phantoms from a tiny cc?
        const auto check_all_in_same_component =
            [](const std::vector<std::pair<PhantomNode, PhantomNode>> &nodes) {
                if (nodes.empty())
                {
                    return true;
                }

                const auto component_id = nodes.front().first.component.id;

                return std::all_of(std::begin(nodes),
                                   std::end(nodes),
                                   [component_id](const PhantomNodePair &phantom_pair) {
                                       return component_id == phantom_pair.first.component.id;
                                   });
            };

        const auto fallback_to_big_component =
            [](const std::pair<PhantomNode, PhantomNode> &phantom_pair) {
                if (phantom_pair.first.component.is_tiny && phantom_pair.second.IsValid() &&
                    !phantom_pair.second.component.is_tiny)
                {
                    return phantom_pair.second;
                }
                return phantom_pair.first;
            };

        const auto use_closed_phantom = [](
            const std::pair<PhantomNode, PhantomNode> &phantom_pair) { return phantom_pair.first; };

        const bool every_phantom_is_in_tiny_cc = std::all_of(std::begin(phantom_node_pair_list),
                                                             std::end(phantom_node_pair_list),
                                                             check_component_id_is_tiny);
        auto all_in_same_component = check_all_in_same_component(phantom_node_pair_list);

        std::vector<PhantomNode> snapped_phantoms;
        snapped_phantoms.reserve(phantom_node_pair_list.size());

        // The only case we don't snap to the big component if all phantoms are in the same small
        // component
        if (every_phantom_is_in_tiny_cc && all_in_same_component)
        {
            std::transform(phantom_node_pair_list.begin(),
                           phantom_node_pair_list.end(),
                           std::back_inserter(snapped_phantoms),
                           use_closed_phantom);
        }
        else
        {
            std::transform(phantom_node_pair_list.begin(),
                           phantom_node_pair_list.end(),
                           std::back_inserter(snapped_phantoms),
                           fallback_to_big_component);
        }

        return snapped_phantoms;
    }

    // Falls back to default_radius for non-set radii
    std::vector<std::vector<PhantomNodeWithDistance>>
    GetPhantomNodesInRange(const api::BaseParameters &parameters,
                           const std::vector<double> radiuses) const
    {
        std::vector<std::vector<PhantomNodeWithDistance>> phantom_nodes(
            parameters.coordinates.size());
        BOOST_ASSERT(radiuses.size() == parameters.coordinates.size());

        const bool use_hints = !parameters.hints.empty();
        const bool use_bearings = !parameters.bearings.empty();

        for (const auto i : util::irange<std::size_t>(0UL, parameters.coordinates.size()))
        {
            if (use_hints && parameters.hints[i] &&
                parameters.hints[i]->IsValid(parameters.coordinates[i], facade))
            {
                phantom_nodes[i].push_back(PhantomNodeWithDistance{
                    parameters.hints[i]->phantom,
                    util::coordinate_calculation::haversineDistance(
                        parameters.coordinates[i], parameters.hints[i]->phantom.location),
                });
                continue;
            }
            if (use_bearings && parameters.bearings[i])
            {
                phantom_nodes[i] =
                    facade.NearestPhantomNodesInRange(parameters.coordinates[i],
                                                      radiuses[i],
                                                      parameters.bearings[i]->bearing,
                                                      parameters.bearings[i]->range);
            }
            else
            {
                phantom_nodes[i] =
                    facade.NearestPhantomNodesInRange(parameters.coordinates[i], radiuses[i]);
            }
        }

        return phantom_nodes;
    }

    std::vector<std::vector<PhantomNodeWithDistance>>
    GetPhantomNodes(const api::BaseParameters &parameters, unsigned number_of_results)
    {
        std::vector<std::vector<PhantomNodeWithDistance>> phantom_nodes(
            parameters.coordinates.size());

        const bool use_hints = !parameters.hints.empty();
        const bool use_bearings = !parameters.bearings.empty();
        const bool use_radiuses = !parameters.radiuses.empty();

        BOOST_ASSERT(parameters.IsValid());
        for (const auto i : util::irange<std::size_t>(0UL, parameters.coordinates.size()))
        {
            if (use_hints && parameters.hints[i] &&
                parameters.hints[i]->IsValid(parameters.coordinates[i], facade))
            {
                phantom_nodes[i].push_back(PhantomNodeWithDistance{
                    parameters.hints[i]->phantom,
                    util::coordinate_calculation::haversineDistance(
                        parameters.coordinates[i], parameters.hints[i]->phantom.location),
                });
                continue;
            }

            if (use_bearings && parameters.bearings[i])
            {
                if (use_radiuses && parameters.radiuses[i])
                {
                    phantom_nodes[i] = facade.NearestPhantomNodes(parameters.coordinates[i],
                                                                  number_of_results,
                                                                  *parameters.radiuses[i],
                                                                  parameters.bearings[i]->bearing,
                                                                  parameters.bearings[i]->range);
                }
                else
                {
                    phantom_nodes[i] = facade.NearestPhantomNodes(parameters.coordinates[i],
                                                                  number_of_results,
                                                                  parameters.bearings[i]->bearing,
                                                                  parameters.bearings[i]->range);
                }
            }
            else
            {
                if (use_radiuses && parameters.radiuses[i])
                {
                    phantom_nodes[i] = facade.NearestPhantomNodes(
                        parameters.coordinates[i], number_of_results, *parameters.radiuses[i]);
                }
                else
                {
                    phantom_nodes[i] =
                        facade.NearestPhantomNodes(parameters.coordinates[i], number_of_results);
                }
            }

            // we didn't find a fitting node, return error
            if (phantom_nodes[i].empty())
            {
                break;
            }
        }
        return phantom_nodes;
    }

    std::vector<PhantomNodePair> GetPhantomNodes(const api::BaseParameters &parameters)
    {
        std::vector<PhantomNodePair> phantom_node_pairs(parameters.coordinates.size());

        const bool use_hints = !parameters.hints.empty();
        const bool use_bearings = !parameters.bearings.empty();
        const bool use_radiuses = !parameters.radiuses.empty();

        BOOST_ASSERT(parameters.IsValid());
        for (const auto i : util::irange<std::size_t>(0UL, parameters.coordinates.size()))
        {
            if (use_hints && parameters.hints[i] &&
                parameters.hints[i]->IsValid(parameters.coordinates[i], facade))
            {
                phantom_node_pairs[i].first = parameters.hints[i]->phantom;
                // we don't set the second one - it will be marked as invalid
                continue;
            }

            if (use_bearings && parameters.bearings[i])
            {
                if (use_radiuses && parameters.radiuses[i])
                {
                    phantom_node_pairs[i] =
                        facade.NearestPhantomNodeWithAlternativeFromBigComponent(
                            parameters.coordinates[i],
                            *parameters.radiuses[i],
                            parameters.bearings[i]->bearing,
                            parameters.bearings[i]->range);
                }
                else
                {
                    phantom_node_pairs[i] =
                        facade.NearestPhantomNodeWithAlternativeFromBigComponent(
                            parameters.coordinates[i],
                            parameters.bearings[i]->bearing,
                            parameters.bearings[i]->range);
                }
            }
            else
            {
                if (use_radiuses && parameters.radiuses[i])
                {
                    phantom_node_pairs[i] =
                        facade.NearestPhantomNodeWithAlternativeFromBigComponent(
                            parameters.coordinates[i], *parameters.radiuses[i]);
                }
                else
                {
                    phantom_node_pairs[i] =
                        facade.NearestPhantomNodeWithAlternativeFromBigComponent(
                            parameters.coordinates[i]);
                }
            }

            // we didn't find a fitting node, return error
            if (!phantom_node_pairs[i].first.IsValid(facade.GetNumberOfNodes()))
            {
                // TODO document why?
                phantom_node_pairs.pop_back();
                break;
            }
            BOOST_ASSERT(phantom_node_pairs[i].first.IsValid(facade.GetNumberOfNodes()));
            BOOST_ASSERT(phantom_node_pairs[i].second.IsValid(facade.GetNumberOfNodes()));
        }
        return phantom_node_pairs;
    }
};
}
}
}

#endif /* BASE_PLUGIN_HPP */
