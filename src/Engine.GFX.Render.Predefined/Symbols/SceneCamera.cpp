#include "SceneCamera.hpp"
#include "../Descriptions/SceneCameraBuffer.hpp"

#include <Engine.Common/Make.hpp>

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
