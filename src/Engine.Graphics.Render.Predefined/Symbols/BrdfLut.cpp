#include "BrdfLut.hpp"

#include <Engine.Common/__macro.hpp>

#include "../Descriptions/BrdfLutBuffer.hpp"

import Engine.Common;

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeBrdfLutSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "brdf::lut",
        makeBrdfLutBufferDescription()
    );
    return obj;
}
