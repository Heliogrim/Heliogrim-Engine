#include "BrdfIrradiance.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/BrdfIrradianceBuffer.hpp"

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeBrdfIrradianceSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "brdf::irradiance",
        makeBrdfIrradianceBufferDescription()
    );
    return obj;
}
