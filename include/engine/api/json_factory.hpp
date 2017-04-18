#ifndef ENGINE_RESPONSE_OBJECTS_HPP_
#define ENGINE_RESPONSE_OBJECTS_HPP_

#include "extractor/guidance/turn_instruction.hpp"
#include "extractor/travel_mode.hpp"
#include "engine/guidance/leg_geometry.hpp"
#include "engine/guidance/route.hpp"
#include "engine/guidance/route_leg.hpp"
#include "engine/guidance/route_step.hpp"
#include "engine/guidance/step_maneuver.hpp"
#include "engine/polyline_compressor.hpp"
#include "util/coordinate.hpp"
#include "util/json_container.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace osrm
{
namespace engine
{

struct Hint;

namespace api
{
namespace json
{
namespace detail
{

std::string instructionTypeToString(extractor::guidance::TurnType::Enum type);
std::string instructionModifierToString(extractor::guidance::DirectionModifier::Enum modifier);

util::json::Array coordinateToLonLat(const util::Coordinate coordinate);

std::string modeToString(const extractor::TravelMode mode);

} // namespace detail

template <typename ForwardIter> util::json::String makePolyline(ForwardIter begin, ForwardIter end)
{
    return {encodePolyline(begin, end)};
}

template <typename ForwardIter>
util::json::Object makeGeoJSONGeometry(ForwardIter begin, ForwardIter end)
{
    auto num_coordinates = std::distance(begin, end);
    BOOST_ASSERT(num_coordinates != 0);
    util::json::Object geojson;
    if (num_coordinates > 1)
    {
        geojson.values["type"] = "LineString";
        util::json::Array coordinates;
        coordinates.values.reserve(num_coordinates);
        std::transform(
            begin, end, std::back_inserter(coordinates.values), &detail::coordinateToLonLat);
        geojson.values["coordinates"] = std::move(coordinates);
    }
    else if (num_coordinates > 0)
    {
        geojson.values["type"] = "Point";
        util::json::Array coordinates;
        coordinates.values.push_back(detail::coordinateToLonLat(*begin));
        geojson.values["coordinates"] = std::move(coordinates);
    }
    return geojson;
}

template <typename ForwardIter>
util::json::Array makeCoordVec1DGeometry(ForwardIter begin, ForwardIter end)
{
    util::json::Array coord_array;
    std::for_each(begin, end, [&coord_array](util::Coordinate const &coord) {
        coord_array.values.push_back(static_cast<double>(toFloating(coord.lat)));
        coord_array.values.push_back(static_cast<double>(toFloating(coord.lon)));
    });
    return coord_array;
}

template <typename T> util::json::Array makeStringArray(const std::vector<T> &vector)
{
    util::json::Array array;
    for (auto const &e : vector)
    {
        array.values.emplace_back(boost::lexical_cast<std::string>(e));
    }
    return array;
}

util::json::Object makeStepManeuver(const guidance::StepManeuver &maneuver);

util::json::Object makeRouteStep(guidance::RouteStep step,
                                 boost::optional<util::json::Value> geometry);

util::json::Object makeRoute(const guidance::Route &route,
                             util::json::Array legs,
                             boost::optional<util::json::Value> geometry,
                             boost::optional<util::json::Value> osm_node_ids);

util::json::Object
makeWaypoint(const util::Coordinate location, std::string name, const Hint &hint);

util::json::Object makeRouteLeg(guidance::RouteLeg leg, util::json::Array steps);

util::json::Array makeRouteLegs(std::vector<guidance::RouteLeg> legs,
                                std::vector<util::json::Value> step_geometries,
                                std::vector<util::json::Object> annotations);
}
}
} // namespace engine
} // namespace osrm

#endif // ENGINE_GUIDANCE_API_RESPONSE_GENERATOR_HPP_
