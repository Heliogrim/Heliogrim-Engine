#include "LeafWidget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

LeafWidget::LeafWidget(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)) {}

LeafWidget::~LeafWidget() = default;

const ptr<const NullChildren> LeafWidget::children() const {
	return get_null_children();
}

math::fvec2 LeafWidget::computeReferenceSize(ReflowAxis axis_) const {
	return math::fvec2 { 0.F };
}

void LeafWidget::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	// __noop();
}

void LeafWidget::applyLayout(ref<ReflowState> state_) {
	// __noop();
}
