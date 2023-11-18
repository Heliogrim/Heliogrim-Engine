#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <tl/optional.hpp>

namespace hg::engine::gfx::acc {
    class EffectPassRemapping final {
    public:
        using this_type = EffectPassRemapping;

        using RemappedSymbol = smr<const Symbol>;
        using AliasSymbol = smr<const Symbol>;

    public:
        EffectPassRemapping() noexcept = default;

        EffectPassRemapping(
            mref<DenseMap<RemappedSymbol, AliasSymbol>> remappedToAlias_,
            mref<DenseMap<AliasSymbol, RemappedSymbol>> aliasToRemapped_
        ) :
            _remappedToAlias(_STD move(remappedToAlias_)),
            _aliasToRemapped(_STD move(aliasToRemapped_)) {}

        EffectPassRemapping(cref<EffectPassRemapping>) noexcept = delete;

        EffectPassRemapping(mref<this_type> other_) noexcept;

        ~EffectPassRemapping() noexcept = default;

    public:
        ref<EffectPassRemapping> operator=(cref<EffectPassRemapping>) noexcept = delete;

        ref<EffectPassRemapping> operator=(mref<EffectPassRemapping> other_) noexcept;

    private:
        DenseMap<RemappedSymbol, AliasSymbol> _remappedToAlias;
        DenseMap<AliasSymbol, RemappedSymbol> _aliasToRemapped;

    public:
        [[nodiscard]] tl::optional<AliasSymbol> alias(mref<RemappedSymbol> remapped_) const noexcept;

        [[nodiscard]] tl::optional<AliasSymbol> alias(cref<TransferToken> remappedToken_) const noexcept;

        [[nodiscard]] tl::optional<RemappedSymbol> remapped(mref<AliasSymbol> alias_) const noexcept;

        [[nodiscard, deprecated]] tl::optional<TransferToken> remappedToken(mref<AliasSymbol> alias_) const noexcept;
    };
}
