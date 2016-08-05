#include <cstdlib>
#include <iostream>
#include <random>

#include "osrm/engine_config.hpp"
#include "osrm/multi_target_parameters.hpp"
#include "osrm/osrm.hpp"
#include "osrm/table_parameters.hpp"

#include "util/coordinate.hpp"
#include "util/json_container.hpp"

using namespace osrm;
using namespace osrm::util;
using namespace osrm::util::json;

std::mt19937 gen;
// std::uniform_real_distribution<> lat_dist{49.379, 51.724};
// std::uniform_real_distribution<> lng_dist{7.696, 10.267};
std::uniform_real_distribution<> lat_dist{47.27, 55.05};
std::uniform_real_distribution<> lng_dist{5.87, 15.03};

FloatCoordinate make_coord(double lat, double lng)
{
    return FloatCoordinate{FloatLongitude{lng}, FloatLatitude{lat}};
}

void eval_forward(OSRM& osrm)
{
    auto const one = make_coord(lat_dist(gen), lng_dist(gen));
    std::vector<FloatCoordinate> many;
    for (int i = 0; i < 1000; ++i)
    {
        many.push_back(make_coord(lat_dist(gen), lng_dist(gen)));
    }

    MultiTargetParameters mt_params;
    mt_params.coordinates.push_back(one);
    std::copy(begin(many), end(many), std::back_inserter(mt_params.coordinates));

    json::Object mt_result;
    auto const mt_status = osrm.MultiTarget(mt_params, mt_result);
    if (mt_status != Status::Ok)
    {
        std::cout << "FWD: bad one-to-many status" << std::endl;
        std::exit(1);
    }

    TableParameters tab_params;
    tab_params.coordinates.push_back(one);
    std::copy(begin(many), end(many), std::back_inserter(tab_params.coordinates));
    tab_params.sources.push_back(0);
    for (auto i = 0u; i < many.size(); ++i)
    {
        tab_params.destinations.push_back(i + 1);
    }

    json::Object tab_result;
    auto const tab_status = osrm.Table(tab_params, tab_result);
    if (tab_status != Status::Ok)
    {
        std::cout << "FWD: bad table status" << std::endl;
        std::exit(1);
    }

    auto mt_costs = mt_result.values["costs"].get<Array>().values;
    auto tab_durations = tab_result.values["durations"].get<Array>().values[0].get<Array>().values;

    if (mt_costs.size() != tab_durations.size())
    {
        std::cout << "FWD: result size mismatch: " << mt_costs.size() << " != " << tab_durations.size();
        std::exit(1);
    }

    for (auto i = 0u; i < mt_costs.size(); ++i)
    {
        auto const mt_dur = mt_costs[i].get<Object>().values.at("duration").get<Number>().value;
        auto const tab_dur = tab_durations[i].get<Number>().value;

        if (std::abs(mt_dur - tab_dur) > std::numeric_limits<double>::epsilon())
        {
            std::cout << "FWD: distance mismatch: " << mt_dur << " != " << tab_dur << std::endl;
            std::exit(1);
        }
    }
}

void eval_backward(OSRM& osrm)
{
    auto const one = make_coord(lat_dist(gen), lng_dist(gen));
    std::vector<FloatCoordinate> many;
    for (int i = 0; i < 100; ++i)
    {
        many.push_back(make_coord(lat_dist(gen), lng_dist(gen)));
    }

    MultiTargetParameters mt_params;
    mt_params.forward = false;
    mt_params.coordinates.push_back(one);
    std::copy(begin(many), end(many), std::back_inserter(mt_params.coordinates));

    json::Object mt_result;
    auto const mt_status = osrm.MultiTarget(mt_params, mt_result);
    if (mt_status != Status::Ok)
    {
        std::cout << "BWD: bad one-to-many status" << std::endl;
        std::exit(1);
    }

    TableParameters tab_params;
    tab_params.coordinates.push_back(one);
    std::copy(begin(many), end(many), std::back_inserter(tab_params.coordinates));
    tab_params.destinations.push_back(0);
    for (auto i = 0u; i < many.size(); ++i)
    {
        tab_params.sources.push_back(i + 1);
    }

    json::Object tab_result;
    auto const tab_status = osrm.Table(tab_params, tab_result);
    if (tab_status != Status::Ok)
    {
        std::cout << "BWD: bad table status" << std::endl;
        std::exit(1);
    }

    auto mt_costs = mt_result.values["costs"].get<Array>().values;
    auto tab_durations = tab_result.values["durations"].get<Array>().values;

    if (mt_costs.size() != tab_durations.size())
    {
        std::cout << "BWD: result size mismatch: " << mt_costs.size() << " != " << tab_durations.size();
        std::exit(1);
    }

    for (auto i = 0u; i < mt_costs.size(); ++i)
    {
        auto const mt_dur = mt_costs[i].get<Object>().values.at("duration").get<Number>().value;
        auto const tab_dur = tab_durations[i].get<Array>().values[0].get<Number>().value;

        if (std::abs(mt_dur - tab_dur) > std::numeric_limits<double>::epsilon())
        {
            std::cout << "distance mismatch: " << mt_dur << " != " << tab_dur << std::endl;
            std::exit(1);
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: ./osrm-eval /path/to/dataset.osrm" << std::endl;
        return 1;
    }

    EngineConfig config;
    config.storage_config = {argv[1]};
    config.use_shared_memory = false;

    OSRM osrm{config};

    for (int x = 0; x < 100; ++x)
    {
        std::cout << "test" << x << std::endl;
        eval_forward(osrm);
        eval_backward(osrm);
    }

    std::cout << "success" << std::endl;

    return 0;
}
