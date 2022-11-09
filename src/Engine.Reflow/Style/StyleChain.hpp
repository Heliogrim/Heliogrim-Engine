#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "StyleChainEntry.hpp"

namespace ember::engine::reflow {

    class StyleChain {
    public:
        template <typename>
        friend class StyleResolver;

    public:
        using this_type = StyleChain;

    public:
        StyleChain();

        ~StyleChain();

    private:
        Vector<StyleChainEntry> _list;

    public:
        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] u64 size() const noexcept;

    public:
        void pushStyle(mref<StyleChainEntry> style_);

        void pushStyle(mref<StyleChainEntry> style_, const s32 off_);

        void popStyle(cref<StyleChainEntry::key_type> style_);
    };

}
