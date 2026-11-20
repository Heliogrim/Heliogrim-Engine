#include "SceneView.hpp"

#include "../Descriptions/SceneView.hpp"

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeSceneViewSymbol() {

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
