#include "DeferredNormal.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/V4F32DataBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Symbol> engine::gfx::render::makeDeferredNormalSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "deferred::normal",
        makeV4F32DataBufferDescription()
    );
    return obj;
}
