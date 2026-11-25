#include "VScrollBox.hpp"

#include <Engine.Common/Math/MathDefaultDefine.hpp>

import Engine.Logging;
import Engine.Reflow;

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

VScrollBox::VScrollBox() :
	VerticalLayout(),
	_scrollTrack(),
	_scrollThumb(),
	_scrollValue(),
	_overflow() {}

VScrollBox::~VScrollBox() = default;

string VScrollBox::getTag() const noexcept {
	return std::format(R"(VScrollBox <{:#x}>)", reinterpret_cast<u64>(this));
}

void VScrollBox::setScrollTrack(cref<sptr<Widget>> track_) {

	track_->setParent(shared_from_this());

	if (_scrollTrack && _scrollTrack->parent().get() == this) {
		_scrollTrack->setParent(nullptr);
	}

	_scrollTrack = track_;
	markAsPending(true, true);
}

void VScrollBox::setScrollThumb(cref<sptr<Widget>> thumb_) {

	thumb_->setParent(shared_from_this());

	if (_scrollThumb && _scrollThumb->parent().get() == this) {
		_scrollThumb->setParent(nullptr);
	}

	_scrollThumb = thumb_;
	markAsPending(true, true);
}

f32 VScrollBox::getScrollValue() const noexcept {
	return _scrollValue;
}

void VScrollBox::scrollTo(cref<math::vec2> point_, cref<math::vec2> size_) {

	const math::vec2 off { _layoutState.layoutOffset };
	const math::vec2 size { _layoutState.layoutSize };

	if (point_.y < off.y) {

		const math::vec2 diff { point_ - off };

		_scrollValue += (diff.y / _overflow);// We expect diff.y to be negative
		_scrollValue = math::clamp(_scrollValue, 0.F, 1.F);

	} else if (point_.y < off.y + size.y) {

		const math::vec2 inner { point_ - off };
		const math::vec2 diff { (point_ + size_) - (off + size) };

		if (diff.y > 0.F && inner.y > 0.F) {

			const float correct { MAX(diff.y, inner.y) };

			_scrollValue += (correct / _overflow);// We expect diff.y to be positive
			_scrollValue = math::clamp(_scrollValue, 0.F, 1.F);

		}

	} else /* if (point_.y >= off.y + size.y) */ {

		const math::vec2 diff { (point_ + size_) - (off + size) };

		_scrollValue += (diff.y / _overflow);// We expect diff.y to be positive
		_scrollValue = math::clamp(_scrollValue, 0.F, 1.F);

	}
}

void VScrollBox::render(const ptr<ReflowCommandBuffer> cmd_) {

	const auto padding = this->get<attr::BoxLayout::padding>();

	const auto off = _layoutState.layoutOffset + math::fvec2 { padding.x, padding.y };
	const auto size = _layoutState.layoutSize - math::fvec2 { padding.x + padding.z, padding.y + padding.w };

	const math::Extent2f32 scissor { size.x, size.y, off.x, off.y };
	cmd_->pushIntersectScissor(scissor);

	/**/

	VerticalLayout::render(cmd_);

	/**/

	if (_scrollTrack) {
		_scrollTrack->render(cmd_);
	}

	if (_scrollThumb) {
		_scrollThumb->render(cmd_);
	}

	/**/

	cmd_->popScissor();
}

PrefetchSizing VScrollBox::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {

	const auto min = algorithm::unitAbsMin(this->get<attr::BoxLayout::minWidth>(), this->get<attr::BoxLayout::minHeight>());
	const auto max = algorithm::unitAbsMax(this->get<attr::BoxLayout::maxWidth>(), this->get<attr::BoxLayout::maxHeight>());

	auto layout = VerticalLayout::prefetchSizing(axis_, state_);
	layout.minSizing.y = std::clamp(0.F, min.y, max.y);
	return layout;
}

void VScrollBox::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	return VerticalLayout::computeSizing(axis_, passState_);
}

void VScrollBox::applyLayout(ref<ReflowState> state_) {

	/**/

	const auto gaps = _children.empty() ? 0uLL : _children.size() - 1uLL;
	auto mayOccupiedSpace = this->get<attr::BoxLayout::padding>().x +
		this->get<attr::BoxLayout::padding>().y +
		this->get<attr::FlexLayout::rowGap>() * gaps;

	for (const auto& child : *children()) {
		mayOccupiedSpace += child->getLayoutState().computeSize.y;
	}

	_overflow = std::max(mayOccupiedSpace - getLayoutState().layoutSize.y, 0.F);

	/**/

	// Warning: This is a bit hacky and unclean
	const auto preserveOffset = getLayoutState().layoutOffset;
	getLayoutState().layoutOffset.y -= (_scrollValue * _overflow);

	algorithm::layout(
		{
			.mainAxis = ReflowAxis::eYAxis,
			.anchor = getLayoutState().layoutOffset,
			.span = getLayoutState().layoutSize,
			.gapping = { this->get<attr::FlexLayout::colGap>(), this->get<attr::FlexLayout::rowGap>() },
			.padding = this->get<attr::BoxLayout::padding>(),
			.align = this->get<attr::FlexLayout::align>(),
			.justify = this->get<attr::FlexLayout::justify>()
		},
		children()
	);

	// Warning: This is the offset restoration corresponding to the hack above
	getLayoutState().layoutOffset = preserveOffset;
}

EventReply VScrollBox::invokeOnWheel(ref<const WheelEvent> event_) {

	if (_children.empty()) {
		_scrollValue = 0.F;
		return VerticalLayout::invokeOnWheel(event_);
	}

	if (_overflow <= 0.F || event_._value.y == 0.F) {
		// Nothing to scroll; dividing by a zero overflow would turn the scroll value into inf or NaN.
		return VerticalLayout::invokeOnWheel(event_);
	}

	constexpr float scrollInvSpeed = 32.F;
	const auto diff { event_._value * (/* _overflow /  */scrollInvSpeed) };
	const auto normalized { diff.y / _overflow };

	_scrollValue -= normalized;
	_scrollValue = math::clamp(_scrollValue, 0.F, 1.F);

	markAsPending(true, false);
	return EventResponse::eConsumed;
}
