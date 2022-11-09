#include "StyleCondition.hpp"

#include "../Widget/Widget.hpp"

using namespace ember::engine::reflow;
using namespace ember;

bool style::isHovered(cref<sptr<Widget>> widget_) {
    return widget_->state().hover;
}

bool style::isFocused(cref<sptr<Widget>> widget_) {
    return widget_->state().focus;
}

bool style::isRaised(cref<sptr<Widget>> widget_) {
    const auto& state { widget_->state() };
    return state.hover || state.focus;
}

bool style::isNever(cref<sptr<Widget>>) {
    return false;
}

bool style::isEach(cref<sptr<Widget>>) {
    return true;
}
