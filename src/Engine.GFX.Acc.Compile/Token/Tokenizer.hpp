#pragma once

#include <Engine.GFX.Acc/Layout/InputLayoutDescription.hpp>
#include <Engine.GFX.Acc/Layout/BindingLayoutDescription.hpp>
#include <Engine.GFX.Acc/Layout/OutputLayoutDescription.hpp>
#include <Engine.GFX.Acc/AccelerationStageInput.hpp>
#include <Engine.GFX.Acc/AccelerationStageOutput.hpp>

#include "Token.hpp"

namespace hg::engine::gfx::acc {
    class Tokenizer {
    public:
        using this_type = Tokenizer;

    public:
        constexpr Tokenizer() noexcept = default;

        Tokenizer(
            mref<string> prefix_,
            mref<string> inScope_,
            mref<string> bindScope_,
            mref<string> outScope_
        ) noexcept;

        ~Tokenizer() noexcept = default;

    private:
        string _prefix { "$" };
        string _inScope { "in" };
        string _bindScope { "material" };
        string _outScope { "out" };

    public:
        [[nodiscard]] Token generate(cref<InputLayoutDescription> ild_) const;

        [[nodiscard]] Token generate(cref<BindingLayoutDescription> bld_) const;

        [[nodiscard]] Token generate(cref<OutputLayoutDescription> old_) const;

    public:
        [[nodiscard]] Token generate(cref<AccelerationStageInput> asi_) const;

        [[nodiscard]] Token generate(cref<AccelerationStageOutput> aso_) const;
    };
}
