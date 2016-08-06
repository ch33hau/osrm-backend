/*
Copyright (c) 2016, Project OSRM contributors
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

#ifndef SMOOTH_VIA_PLUGIN_H
#define SMOOTH_VIA_PLUGIN_H

#include "engine/api/smooth_via_parameters.hpp"
#include "engine/datafacade/datafacade_base.hpp"
#include "engine/plugins/plugin_base.hpp"

#include "engine/routing_algorithms/direct_shortest_path.hpp"
#include "engine/routing_algorithms/shortest_path.hpp"
#include "engine/search_engine_data.hpp"
#include "util/json_container.hpp"

namespace osrm
{
namespace engine
{
namespace plugins
{

struct LegResult
{
    double duration;
    double distance;
    std::vector<Coordinate> polyline;
};

class SmoothViaPlugin final : public BasePlugin
{
  private:
    SearchEngineData heaps;
    routing_algorithms::DirectShortestPathRouting<datafacade::BaseDataFacade> direct_shortest_path;
    routing_algorithms::ShortestPathRouting<datafacade::BaseDataFacade> shortest_path;

  public:
    explicit SmoothViaPlugin(datafacade::BaseDataFacade &facade);

    Status HandleRequest(const api::SmoothViaParameters &params, util::json::Object &result);

  private:
    std::vector<std::vector<PhantomNode>> ResolveNodes(const api::SmoothViaParameters &);

    std::vector<std::vector<std::vector<LegResult>>>
    RouteAllLegs(const std::vector<std::vector<PhantomNode>> &);

    LegResult RouteDirect(const PhantomNode &from, const PhantomNode &to);
};
}
}
}

#endif // SMOOTH_VIA_PLUGIN_H
