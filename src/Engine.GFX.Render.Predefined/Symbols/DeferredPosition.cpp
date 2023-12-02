#include "DeferredPosition.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/V4F32DataBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Symbol> engine::gfx::render::makeDeferredPositionSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "deferred::position",
        makeV4F32DataBufferDescription()
    );
    return obj;
}
