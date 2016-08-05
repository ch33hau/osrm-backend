#include "engine/plugins/multi_target.hpp"

namespace osrm
{
namespace engine
{
namespace plugins
{

MultiTargetPlugin::MultiTargetPlugin(datafacade::BaseDataFacade &facade_)
    : BasePlugin(facade_), multi_target_forward(&facade_, heaps),
      multi_target_backward(&facade_, heaps)
{
}

Status MultiTargetPlugin::HandleRequest(const api::MultiTargetParameters &parameters,
                                        util::json::Object &json_object)
{
    if (!parameters.IsValid() || std::any_of(begin(parameters.coordinates),
                                             end(parameters.coordinates),
                                             [](Coordinate c) { return !c.IsValid(); }))
    {
        return Status::Error;
    }

    auto snapped_phantoms = SnapPhantomNodes(GetPhantomNodes(parameters));

    std::shared_ptr<std::vector<std::pair<double, double>>> result_table;
    if (parameters.forward)
    {
        result_table = multi_target_forward(snapped_phantoms);
    }
    else
    {
        result_table = multi_target_backward(snapped_phantoms);
    }

    if (!result_table)
    {
        return Status::Error;
    }

    util::json::Array json_array;
    for (unsigned column = 0; column < parameters.coordinates.size() - 1; ++column)
    {
        auto routing_result = result_table->operator[](column);

        util::json::Object result;
        result.values["duration"] = routing_result.first;
        result.values["distance"] = routing_result.second;
        json_array.values.emplace_back(result);
    }
    json_object.values["costs"] = json_array;

    return Status::Ok;
}
}
}
}
