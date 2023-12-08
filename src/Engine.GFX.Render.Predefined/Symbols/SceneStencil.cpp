#include "SceneStencil.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/StencilBuffer.hpp"

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
