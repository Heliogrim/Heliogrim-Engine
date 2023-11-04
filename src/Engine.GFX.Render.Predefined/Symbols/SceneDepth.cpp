#include "SceneDepth.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/DepthBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSceneDepthSymbol() {

    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::depth",
        makeDepthBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
