#pragma once

#include <Engine.GFX.Acc/Stage/StageInput.hpp>
#include <Engine.GFX.Acc/Stage/StageOutput.hpp>

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
        [[nodiscard]] Token generate(cref<StageInput> asi_) const;

        [[nodiscard]] Token generate(cref<StageOutput> aso_) const;

    public:
        [[nodiscard]] Token transformAccStageIn(cref<Token> src_, bool forwarding) const;

        [[nodiscard]] Token transformAccStageOut(cref<Token> src_, bool forwarding) const;

        [[nodiscard]] Token transformAccStageInToOut(cref<Token> src_) const;

    public:
        [[nodiscard]] bool isStageIn(cref<Token> token_, bool forwarding) const;

        [[nodiscard]] bool isStageOut(cref<Token> token_, bool forwarding) const;
    };
}
