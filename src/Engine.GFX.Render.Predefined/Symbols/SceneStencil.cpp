#include "SceneStencil.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/StencilBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSceneStencilSymbol() {

    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::stencil",
        makeStencilBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
