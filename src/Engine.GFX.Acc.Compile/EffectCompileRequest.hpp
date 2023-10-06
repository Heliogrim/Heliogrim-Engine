#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>

#include "Profile/EffectProfile.hpp"
#include "Spec/SpecificationStorage.hpp"

namespace hg::engine::gfx::acc {
    struct EffectCompileRequest final {
        smr<AccelerationEffect> effect;
        smr<const EffectProfile> profile;
        cref<SpecificationStorage> spec;

        /**
         * @details The targeted symbols to export. Primarly used for optimization
         *  and trimming of the provided effect.
         */
        Vector<smr<const Symbol>> targetSymbols;
    };
}
