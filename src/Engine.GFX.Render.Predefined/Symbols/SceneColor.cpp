#include "SceneColor.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/ColorBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Symbol> engine::gfx::render::makeSceneColorSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::color",
        makeColorBufferDescription()
    );
    return obj;
}
