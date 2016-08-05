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

#ifndef MULTI_TARGET_PLUGIN_H
#define MULTI_TARGET_PLUGIN_H

#include "engine/api/multi_target_parameters.hpp"
#include "engine/datafacade/datafacade_base.hpp"
#include "engine/plugins/plugin_base.hpp"

#include "engine/routing_algorithms/multi_target.hpp"
#include "engine/search_engine_data.hpp"
#include "util/json_container.hpp"

namespace osrm
{
namespace engine
{
namespace plugins
{

class MultiTargetPlugin final : public BasePlugin
{
  private:
    SearchEngineData heaps;
    routing_algorithms::MultiTargetRouting<datafacade::BaseDataFacade, true> multi_target_forward;
    routing_algorithms::MultiTargetRouting<datafacade::BaseDataFacade, false> multi_target_backward;

  public:
    explicit MultiTargetPlugin(datafacade::BaseDataFacade &facade);

    Status HandleRequest(const api::MultiTargetParameters &parameters,
                         util::json::Object &json_result);
};
}
}
}

#endif // MULTI_TARGET_PLUGIN_H
