#pragma once

#include <Engine.Common/Managed/SharedFromThis.hpp>

#include "StyleChain.hpp"
#include "StyleKeyStack.hpp"

namespace ember::engine::reflow {
    class BoundStyleSheet :
        public StyleSheet,
        public SharedFromThis<BoundStyleSheet> {
    public:
        using this_type = BoundStyleSheet;

    private:
        BoundStyleSheet();

        BoundStyleSheet(cref<StyleSheet> local_);

        BoundStyleSheet(mref<StyleSheet> local_);

        BoundStyleSheet(cref<sptr<StyleSheet>> local_);

        BoundStyleSheet(mref<this_type> other_);

    public:
        ~BoundStyleSheet();

    public:
        [[nodiscard]] static sptr<BoundStyleSheet> make();

        [[nodiscard]] static sptr<BoundStyleSheet> make(cref<StyleSheet> local_);

        [[nodiscard]] static sptr<BoundStyleSheet> make(mref<StyleSheet> local_);

        [[nodiscard]] static sptr<BoundStyleSheet> make(cref<sptr<StyleSheet>> local_);

        [[nodiscard]] static sptr<BoundStyleSheet> make(mref<this_type> local_);

    private:
        void sanitize();

        void sanitizeMove();

    private:
        StyleChain _styleChain;

    public:
        void pushStyle(mref<StyleChainEntry> style_);

        void popStyle(cref<StyleChainEntry::key_type> style_);

    public:
        StyleSheet compute(cref<sptr<Widget>> widget_, _Inout_ ref<StyleKeyStack> styleStack_);
    };
}
