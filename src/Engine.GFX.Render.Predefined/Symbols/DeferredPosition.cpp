#include "DeferredPosition.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/V4F32DataBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeDeferredPositionSymbol() {
    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "deferred::position",
        makeV4F32DataBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
