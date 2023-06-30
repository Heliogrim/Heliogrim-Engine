#pragma once

#include <Engine.GFX.Acc/Effect/InputLayoutDescription.hpp>
#include <Engine.GFX.Acc/Effect/BindingLayoutDescription.hpp>
#include <Engine.GFX.Acc/Effect/OutputLayoutDescription.hpp>
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
        string _bindScope { "bind" };
        string _outScope { "out" };

    private:
        [[nodiscard]] Token expand(mref<Token> token_, cref<string> data_) const;

    public:
        [[nodiscard]] Token generate(cref<InputLayoutDescription> ild_) const;

        [[nodiscard]] Token generate(cref<BindingLayoutDescription> bld_) const;

        [[nodiscard]] Token generate(
            cref<BindingLayoutDescription> bld_,
            cref<BindingLayoutAttributeDescription> blad_
        ) const;

        [[nodiscard]] Token generate(cref<OutputLayoutDescription> old_) const;

    public:
        [[nodiscard]] Token generate(cref<AccelerationStageInput> asi_) const;

        [[nodiscard]] Token generate(cref<AccelerationStageOutput> aso_) const;

    public:
        [[nodiscard]] Token transformAccStageIn(cref<Token> src_, bool forwarding, bool dynamic) const;

        [[nodiscard]] Token transformAccStageOut(cref<Token> src_, bool forwarding, bool dynamic) const;

        [[nodiscard]] Token transformAccStageInToOut(cref<Token> src_) const;

    public:
        [[nodiscard]] bool isStageIn(cref<Token> token_, bool forwarding, bool dynamic) const;

        [[nodiscard]] bool isStageOut(cref<Token> token_, bool forwarding, bool dynamic) const;
    };
}
