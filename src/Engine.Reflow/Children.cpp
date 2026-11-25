#include "Children.hpp"

#include "Widget/NullWidget.hpp"

import std;

using namespace hg::engine::reflow;
using namespace hg;

static const NullChildren nullChildrenInstance {};

/**/

SingleChildren::SingleChildren() {
	push_back(NullWidget::instance());
}

SingleChildren::SingleChildren(ref<const sptr<Widget>> nonNullChild_) {
	emplace_back(nonNullChild_ != nullptr ? nonNullChild_ : NullWidget::instance());
}

sptr<Widget> SingleChildren::setChild(cref<sptr<Widget>> nextChild_) {
	return std::exchange(front(), nextChild_);
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

void hg::engine::reflow::replaceSingleChild(ref<Widget> owner_, ref<SingleChildren> children_, const sptr<Widget> content_) {
	if (const auto& previous = children_.getChild(); previous != nullptr && previous != content_) {
		previous->setParent(nullptr);
	}
	if (content_ != nullptr) {
		content_->setParent(owner_.shared_from_this());
	}

	children_.setChild(content_);
	owner_.markAsPending(true, true);
}

/**/

const non_owning_rptr<const NullChildren> engine::reflow::get_null_children() noexcept {
	return &nullChildrenInstance;
}
