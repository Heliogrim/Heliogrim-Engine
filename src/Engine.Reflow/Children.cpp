#include "Children.hpp"

#include "Widget/NullWidget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

static const NullChildren nullChildrenInstance {};

/**/

SingleChildren::SingleChildren() {
    push_back(NullWidget::instance());
}

sptr<Widget> SingleChildren::setChild(cref<sptr<Widget>> nextChild_) {
    return _STD exchange(front(), nextChild_);
}

sptr<Widget> SingleChildren::getChild() const noexcept {
    if (empty()) {
        return NullWidget::instance();
    }
    return front();
}

void SingleChildren::resetChild() {
    clear();
    push_back(NullWidget::instance());
}

/**/

const non_owning_rptr<const NullChildren> engine::reflow::get_null_children() noexcept {
    return &nullChildrenInstance;
}
