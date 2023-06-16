#pragma once

#include <Engine.Common/Collection/Set.hpp>

#include "Token.hpp"

namespace hg::engine::gfx::acc {
    class ScopedTokenStorage {
    public:
        using this_type = ScopedTokenStorage;

    private:
        CompactSet<Token, _STD hash<Token>> _tokens;

    public:
        void addToken(mref<Token> token_);

        [[nodiscard]] bool hasToken(cref<Token> token_) const noexcept;

        void removeToken(cref<Token> token_);
    };
}
