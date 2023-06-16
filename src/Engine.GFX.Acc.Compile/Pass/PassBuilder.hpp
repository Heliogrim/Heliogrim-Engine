#pragma once

#include <Engine.GFX.Acc/AccelerationEffect.hpp>

namespace hg::engine::gfx::acc {
    class __declspec(novtable) PassBuilder {
    public:
        using this_type = PassBuilder;

    public:
        PassBuilder() noexcept = default;

        virtual ~PassBuilder() noexcept = default;

    public:
        [[nodiscard]] _Success_(return != nullptr) virtual smr<AccelerationPass> build(
            mref<smr<AccelerationEffect>> effect_
        ) const noexcept = 0;
    };
}
