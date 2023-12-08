#include "SceneColor.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/ColorBuffer.hpp"

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeSceneColorSymbol() {
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
