#include "SceneCamera.hpp"

#include <Engine.Common/__macro.hpp>

#include "../Descriptions/SceneCameraBuffer.hpp"

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeSceneCameraSymbol() {

    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::camera",
        makeSceneCameraBufferDescription()
    );
    return obj;
}
