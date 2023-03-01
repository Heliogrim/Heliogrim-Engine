#include "StyleCondition.hpp"

#include "../Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

bool style::isHovered(cref<sptr<Widget>> widget_) {
    return widget_->state().isHover();
}

bool style::isFocused(cref<sptr<Widget>> widget_) {
    return widget_->state().isFocus();
}

bool style::isRaised(cref<sptr<Widget>> widget_) {
    const auto& state { widget_->state() };
    return state.isHover() || state.isFocus();
}

bool style::isNever(cref<sptr<Widget>>) {
    return false;
}

bool style::isEach(cref<sptr<Widget>>) {
    return true;
}
