#include "DirectionalShadow.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/DepthBuffer.hpp"

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeDirectionalShadowSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "shadow::directional",
        makeDepthBufferDescription()
    );
    return obj;
}
