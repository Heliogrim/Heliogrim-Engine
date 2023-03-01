#pragma once

#include "StyleSheetEntry.hpp"

namespace hg::engine::reflow {
    template <bool AggregateSetFlag_ = true>
    class StyleCombinator {
    public:
        using this_type = StyleCombinator;

    public:
        template <typename ValueType_>
        StyleSheetEntry<ValueType_> operator()(
            cref<StyleSheetEntry<ValueType_>> style_,
            cref<StyleSheetEntry<ValueType_>> apply_
        ) const {

            using value_type = StyleSheetEntry<ValueType_>;

            if constexpr (AggregateSetFlag_) {
                return apply_.set ? apply_ : style_;

            } else {
                return value_type {
                    style_.set,
                    apply_.set ? apply_.attr : style_.attr
                };
            }
        }
    };
}
