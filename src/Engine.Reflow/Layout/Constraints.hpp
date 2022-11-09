#pragma once

#include "../Style/StyleKeyStack.hpp"
#include "../Style/StyleSheet.hpp"

namespace ember::engine::reflow::layout {

    [[nodiscard]] extern bool hasStyleChanged(cref<StyleKeyStack::set_type> prev_, cref<StyleKeyStack> next_);

    [[nodiscard]] extern bool hasConstSize(cref<StyleSheet> style_);

}
