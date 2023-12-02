#include "SceneView.hpp"
#include "../Descriptions/SceneView.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Symbol> engine::gfx::render::makeSceneViewSymbol() {

    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::view",
        makeSceneViewDescription()
    );
    return obj;
}
