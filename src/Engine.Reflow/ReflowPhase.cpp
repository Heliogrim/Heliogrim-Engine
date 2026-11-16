#include "ReflowPhase.hpp"

#include "Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

void hg::engine::reflow::notifyAttributeChange(nmpt<Widget> widget_, const Flag<ReflowPhase, u8> phases_) noexcept {

	if (widget_ == nullptr || phases_.unwrap == 0u) {
		return;
	}

	if (widget_->_batchDepth > 0u) {
		widget_->_batchPhases |= phases_;
		return;
	}

	markPhases(*widget_, phases_);
}

void hg::engine::reflow::markPhases(ref<Widget> widget_, const Flag<ReflowPhase, u8> phases_) noexcept {

	if (phases_ & ReflowPhase::eLayout) {
		widget_.markLayoutPending();

	} else if (phases_ & ReflowPhase::ePaint) {
		widget_.markPaintPending();
	}
}
