#include "BrdfPrefilter.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/BrdfPrefilterBuffer.hpp"

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeBrdfPrefilterSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "brdf::prefilter",
        makeBrdfPrefilterBufferDescription()
    );
    return obj;
}
