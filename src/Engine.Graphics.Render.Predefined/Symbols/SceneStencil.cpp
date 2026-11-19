#include "SceneStencil.hpp"

#include <Engine.Common/__macro.hpp>

#include "../Descriptions/StencilBuffer.hpp"

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeSceneStencilSymbol() {

    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::stencil",
        makeStencilBufferDescription()
    );
    return obj;
}
