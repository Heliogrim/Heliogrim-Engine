#pragma once
#include <Engine.GFX.Acc/AccelerationStageDerivat.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>

#include "ModuleSource.hpp"
#include "../Token/ScopedTokenStorage.hpp"

namespace hg::engine::gfx::acc {
    class __declspec(novtable) ModuleBuilder {
    public:
        using this_type = ModuleBuilder;

    public:
        ModuleBuilder() noexcept = default;

        virtual ~ModuleBuilder() noexcept = default;

    public:
        [[nodiscard]] virtual uptr<ModuleSource> build(
            cref<smr<AccelerationPass>> targetPass_,
            cref<ScopedTokenStorage> scopedTokens_,
            cref<smr<AccelerationStageDerivat>> stage_
        ) const = 0;
    };
}
