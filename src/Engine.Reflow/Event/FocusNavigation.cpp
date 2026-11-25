#include "FocusNavigation.hpp"

#include "../Children.hpp"
#include "../Widget/FocusScope.hpp"
#include "../Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

/**/

namespace {
	void collectVisible(ref<const sptr<Widget>> widget_, ref<Vector<sptr<Widget>>> order_);
}

/**/

sptr<FocusScope> hg::engine::reflow::nearestScope(ref<const sptr<Widget>> widget_) {
	for (auto next = widget_; next != nullptr; next = next->parent()) {
		if (auto scope = std::dynamic_pointer_cast<FocusScope>(next)) {
			return scope;
		}
	}
	return nullptr;
}

sptr<Widget> hg::engine::reflow::nextFocusable(
	ref<const sptr<Widget>> scopeRoot_,
	ref<const sptr<Widget>> current_,
	const bool backwards_
) {

	Vector<sptr<Widget>> order {};
	collectVisible(scopeRoot_, order);

	const auto count = static_cast<s64>(order.size());
	if (count == 0LL) {
		return nullptr;
	}

	const auto found = std::ranges::find(order, current_);
	const auto base = found != order.end() ? static_cast<s64>(found - order.begin()) : (backwards_ ? count : -1LL);

	// Bounded to one lap, so a scope without candidates terminates.
	for (s64 step = 1LL; step <= count; ++step) {
		const auto idx = (((base + (backwards_ ? -step : step)) % count) + count) % count;
		if (order[idx]->isFocusable()) {
			return order[idx];
		}
	}
	return nullptr;
}

/**/

namespace {
	void collectVisible(ref<const sptr<Widget>> widget_, ref<Vector<sptr<Widget>>> order_) {
		if (widget_ == nullptr || not widget_->state().isVisible()) {
			return;
		}
		order_.push_back(widget_);
		if (const auto children = widget_->children(); children != nullptr) {
			for (const auto& child : *children) {
				collectVisible(child, order_);
			}
		}
	}
}
