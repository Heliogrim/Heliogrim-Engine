#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Functional/Function.hpp>

#include "StyleSheet.hpp"
#include "../Widget/__fwd.hpp"

namespace hg::engine::reflow {
    class StyleChainEntry {
    public:
        using this_type = StyleChainEntry;

        using key_type = AssocKey<string>;
        using cnd_type = unary_fnc<bool, cref<sptr<Widget>>>;

    public:
        StyleChainEntry();

        StyleChainEntry(cref<key_type> key_, cref<cnd_type> cnd_, cref<sptr<StyleSheet>> style_);

        StyleChainEntry(cref<this_type> other_);

        StyleChainEntry(mref<this_type> other_) noexcept;

        ~StyleChainEntry();

    public:
        ref<this_type> operator=(cref<this_type> other_);

        ref<this_type> operator=(mref<this_type> other_) noexcept;

    private:
        key_type _key;
        cnd_type _cnd;

        sptr<StyleSheet> _style;

    public:
        [[nodiscard]] cref<key_type> key() const noexcept;

        [[nodiscard]] cref<cnd_type> cnd() const noexcept;

        [[nodiscard]] cref<sptr<StyleSheet>> style() const noexcept;
    };
}
