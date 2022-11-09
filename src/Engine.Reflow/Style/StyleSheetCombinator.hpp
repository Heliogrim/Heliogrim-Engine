#pragma once

#include "StyleSheet.hpp"
#include "StyleCombinator.hpp"

namespace ember::engine::reflow {

    template <typename CombinatorType_ = StyleCombinator<false>>
    class StyleSheetCombinator :
        public CombinatorType_ {
    public:
        using this_type = StyleSheetCombinator;
        using combinator_type = CombinatorType_;

    public:
        ref<StyleSheet> operator()(_Inout_ ref<StyleSheet> self_, cref<StyleSheet> apply_) const {

            cref<combinator_type> comb { static_cast<cref<combinator_type>>(*this) };

            #define COMP_ASSIGN(field) self_.field = comb(self_.field, apply_.field)

            COMP_ASSIGN(minWidth);
            COMP_ASSIGN(width);
            COMP_ASSIGN(maxWidth);

            COMP_ASSIGN(minHeight);
            COMP_ASSIGN(height);
            COMP_ASSIGN(maxHeight);

            COMP_ASSIGN(wrap);
            COMP_ASSIGN(colGap);
            COMP_ASSIGN(rowGap);

            COMP_ASSIGN(padding);
            COMP_ASSIGN(margin);

            COMP_ASSIGN(reflowSpacing);
            COMP_ASSIGN(reflowShrink);
            COMP_ASSIGN(reflowGrow);

            COMP_ASSIGN(border);
            COMP_ASSIGN(borderRadius);
            COMP_ASSIGN(borderColor);

            COMP_ASSIGN(color);

            COMP_ASSIGN(font);
            COMP_ASSIGN(fontSize);
            COMP_ASSIGN(lineHeight);
            COMP_ASSIGN(textAlign);
            COMP_ASSIGN(textEllipse);

            #undef COMP_ASSIGN

            return self_;
        }
    };

}
