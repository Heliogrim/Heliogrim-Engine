#include "SceneColor.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/ColorBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSceneColorSymbol() {
    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::color",
        makeColorBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
