#include "ScopedTokenStorage.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

void ScopedTokenStorage::addToken(mref<Token> token_) {
    _tokens.insert(token_);
}

bool ScopedTokenStorage::hasToken(cref<Token> token_) const noexcept {
    return _tokens.contains(token_);
}

void ScopedTokenStorage::removeToken(cref<Token> token_) {
    _tokens.erase(token_);
}
