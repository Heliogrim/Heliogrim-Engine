#pragma once

#include "StyleChain.hpp"
#include "StyleSheetCombinator.hpp"

#include "../Widget/Widget.hpp"

namespace ember::engine::reflow {
    template <typename CombinatorType_ = StyleSheetCombinator<StyleCombinator<false>>>
    class StyleResolver :
        public CombinatorType_ {
    public:
        using this_type = StyleResolver<CombinatorType_>;
        using combinator_type = CombinatorType_;

    public:
        StyleResolver() :
            combinator_type() {}

        ~StyleResolver() noexcept(_STD is_nothrow_destructible_v<combinator_type>) = default;

    public:
        [[nodiscard]] StyleSheet operator()(
            cref<StyleChain> chain_,
            cref<sptr<Widget>> widget_,
            _Inout_ ref<StyleKeyStack> styleStack_,
            cref<StyleSheet> initial_ = StyleSheet {}
        ) const {

            if (chain_.empty()) {
                return initial_;
            }

            StyleSheet style { initial_ };
            for (const auto& entry : chain_._list) {

                const auto& cnd { entry.cnd() };
                if (cnd && not styleStack_.contains(entry.key())) {

                    if (not cnd(widget_)) {
                        continue;
                    }

                    styleStack_.pushKey(entry.key());
                }

                style = (static_cast<cref<combinator_type>>(*this))(style, *(entry.style()));
            }

            return style;
        }
    };
}
