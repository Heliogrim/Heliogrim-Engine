#include "CompoundWidget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

CompoundWidget::CompoundWidget(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	_children() {}

CompoundWidget::CompoundWidget(mref<SharedPtr<Widget>> content_, mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_) :
	Widget(::hg::move(classList_), ::hg::move(parent_)),
	_children(::hg::move(content_)) {}

CompoundWidget::~CompoundWidget() = default;

const ptr<const SingleChildren> CompoundWidget::children() const {
	return &_children;
}

void CompoundWidget::render(const ptr<ReflowCommandBuffer> cmd_) {
	_children.getChild()->render(cmd_);
}

PrefetchSizing CompoundWidget::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return {
		_children.getChild()->getLayoutState().prefetchMinSize,
		_children.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing CompoundWidget::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _children.getChild()->passPrefetchSizing(axis_, passState_);
}

math::fvec2 CompoundWidget::computeReferenceSize(ReflowAxis axis_) const {
	return _layoutState.referenceSize;
}

void CompoundWidget::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_children.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void CompoundWidget::applyLayout(ref<ReflowState> state_) {
	const auto childLayout = state_.getStateOf(_children.getChild());
	childLayout->layoutOffset = _layoutState.layoutOffset;
	childLayout->layoutSize = _layoutState.layoutSize;
}

math::fvec2 CompoundWidget::getShrinkFactor() const noexcept {
	return _children.getChild()->getShrinkFactor();
}

math::fvec2 CompoundWidget::getGrowFactor() const noexcept {
	return _children.getChild()->getGrowFactor();
}
