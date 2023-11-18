#include "EffectPassRemapping.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

EffectPassRemapping::EffectPassRemapping(mref<this_type> other_) noexcept :
    _remappedToAlias(_STD move(other_._remappedToAlias)),
    _aliasToRemapped(_STD move(other_._aliasToRemapped)) {}

ref<EffectPassRemapping> EffectPassRemapping::operator=(mref<EffectPassRemapping> other_) noexcept {
    if (_STD addressof(other_) != this) {
        _remappedToAlias = _STD move(other_._remappedToAlias);
        _aliasToRemapped = _STD move(other_._aliasToRemapped);
    }
    return *this;
}

tl::optional<EffectPassRemapping::AliasSymbol>
EffectPassRemapping::alias(mref<RemappedSymbol> remapped_) const noexcept {
    const auto it = _remappedToAlias.find(remapped_);
    if (it != _remappedToAlias.end()) {
        return it->second;
    }
    return tl::nullopt;
}

tl::optional<EffectPassRemapping::AliasSymbol> EffectPassRemapping::alias(
    cref<TransferToken> remappedToken_
) const noexcept {
    for (const auto& [remapped, alias] : _remappedToAlias) {
        if (remapped->token == remappedToken_) {
            return alias;
        }
    }
    return tl::nullopt;
}

tl::optional<EffectPassRemapping::RemappedSymbol>
EffectPassRemapping::remapped(mref<AliasSymbol> alias_) const noexcept {
    const auto it = _aliasToRemapped.find(alias_);
    if (it != _aliasToRemapped.end()) {
        return it->second;
    }
    return tl::nullopt;
}

tl::optional<TransferToken> EffectPassRemapping::remappedToken(mref<AliasSymbol> alias_) const noexcept {
    const auto it = _aliasToRemapped.find(alias_);
    if (it != _aliasToRemapped.end()) {
        return it->second->token;
    }
    return tl::nullopt;
}
