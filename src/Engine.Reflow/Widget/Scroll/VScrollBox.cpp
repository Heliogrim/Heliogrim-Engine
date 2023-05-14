#include "VScrollBox.hpp"

#include <Engine.Logging/Logger.hpp>
#include "../../Layout/Style.hpp"
#include "../../Algorithm/Flex.hpp"
#include "../../Algorithm/FlexState.hpp"
#include "../../Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

VScrollBox::VScrollBox() :
    VerticalPanel() {}

VScrollBox::~VScrollBox() = default;

string VScrollBox::getTag() const noexcept {
    return _STD format(R"(VScrollBox <{:#x}>)", reinterpret_cast<u64>(this));
}

void VScrollBox::setScrollTrack(cref<sptr<Widget>> track_) {

    track_->setParent(shared_from_this());

    if (_scrollTrack && _scrollTrack->parent().get() == this) {
        _scrollTrack->setParent(nullptr);
    }

    _scrollTrack = track_;
    markAsPending();
}

void VScrollBox::setScrollThumb(cref<sptr<Widget>> thumb_) {

    thumb_->setParent(shared_from_this());

    if (_scrollThumb && _scrollThumb->parent().get() == this) {
        _scrollThumb->setParent(nullptr);
    }

    _scrollThumb = thumb_;
    markAsPending();
}

math::vec2 VScrollBox::getScrollValue() const noexcept {
    return _scrollValue;
}

void VScrollBox::scrollTo(cref<math::vec2> point_, cref<math::vec2> size_) {

    const math::vec2 off { _layoutState.layoutOffset };
    const math::vec2 size { _layoutState.layoutSize };

    if (point_.y < off.y) {

        const math::vec2 diff { point_ - off };

        _scrollValue.y += (diff.y / _overflow.y);// We expect diff.y to be negative
        _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

    } else if (point_.y < off.y + size.y) {

        const math::vec2 inner { point_ - off };
        const math::vec2 diff { (point_ + size_) - (off + size) };

        if (diff.y > 0.F && inner.y > 0.F) {

            const float correct { MAX(diff.y, inner.y) };

            _scrollValue.y += (correct / _overflow.y);// We expect diff.y to be positive
            _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

        }

    } else /* if (point_.y >= off.y + size.y) */ {

        const math::vec2 diff { (point_ + size_) - (off + size) };

        _scrollValue.y += (diff.y / _overflow.y);// We expect diff.y to be positive
        _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

    }
}

void VScrollBox::render(const ptr<ReflowCommandBuffer> cmd_) {

    const auto off = _layoutState.layoutOffset;
    const auto size = _layoutState.layoutSize;

    const math::fExtent2D scissor { size.x, size.y, off.x, off.y };
    cmd_->pushIntersectScissor(scissor);

    /**/

    VerticalPanel::render(cmd_);

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

math::vec2 VScrollBox::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return VerticalPanel::prefetchDesiredSize(state_, scale_);
}

math::vec2 VScrollBox::computeDesiredSize(cref<ReflowPassState> passState_) const {
    return VerticalPanel::computeDesiredSize(passState_);
}

void VScrollBox::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {

    const auto innerSize = layout::outerToInnerSize(attr, ctx_.localSize);

    algorithm::FlexState flexState {};
    flexState.box.preservedSize = innerSize;
    flexState.box.maxSize = innerSize;
    flexState.box.orientation = algorithm::FlexLineOrientation::eVertical;
    flexState.box.justify = attr.justify.getValue();
    flexState.box.align = attr.align.getValue();
    flexState.box.gap = math::vec2 { attr.colGap.getValue(), attr.rowGap.getValue() };
    flexState.box.wrap = false;

    /**/

    algorithm::solve(flexState, state_, children());

    /**/

    const auto minFlexBound = /*ctx_.localOffset*/math::vec2 { 0.F };
    const auto maxFlexBound = /*ctx_.localOffset + ctx_.localSize*/innerSize;

    const auto offset = layout::outerToInnerOffset(attr, ctx_.localOffset);

    /**/

    const auto& lfl = flexState.lines.back();
    const auto fbmax = lfl.offset + lfl.size;
    _overflow = math::compMax<float>(math::vec2 { 0.F }, fbmax - flexState.box.maxSize);

    /**/

    for (const auto& flexLine : flexState.lines) {
        for (const auto& flexItem : flexLine.items) {

            const auto dummy = flexItem.widget.lock();
            const auto widgetState = state_.getStateOf(_STD static_pointer_cast<Widget, void>(dummy));

            widgetState->layoutOffset = flexItem.offset + offset;
            widgetState->layoutSize = flexItem.flexSize;

            // TODO: Check how we should work with co-axis
            // Currently Flex-Solving will guarantee main-axis constraint (as long as possible), but co-axis will break
            // We might try to hard-limit contextual constraints

            math::vec2 minDiff;
            math::vec2 maxDiff;

            const auto minCorner = flexItem.offset;
            const auto maxCorner = flexItem.offset + flexItem.flexSize;

            if (maxCorner.x > maxFlexBound.x || maxCorner.y > maxFlexBound.y) {
                maxDiff = maxCorner - maxFlexBound;
                maxDiff = math::compMax<float>(maxDiff, math::vec2 { 0.F });
            }

            if (minCorner.x > minFlexBound.x || minCorner.y > minFlexBound.y) {
                minDiff = minFlexBound - minCorner;
                minDiff = math::compMax<float>(minDiff, math::vec2 { 0.F });
            }

            widgetState->layoutOffset += minDiff;

            if (algorithm::FlexLineOrientation::eVertical == algorithm::FlexLineOrientation::eVertical) {
                widgetState->layoutSize.x -= maxDiff.x;
            } else {
                widgetState->layoutSize.y -= maxDiff.y;
            }

            widgetState->layoutOffset -= (_scrollValue * _overflow);
        }
    }
}

EventResponse VScrollBox::onWheel(cref<WheelEvent> event_) {

    if (_children.empty()) {
        _scrollValue = math::vec2 { 0.F };
        return VerticalPanel::onWheel(event_);
    }

    constexpr float scrollInvSpeed = 32.F;
    const auto diff { event_._value * (/* _overflow /  */scrollInvSpeed) };
    const auto normalized { diff / _overflow };

    _scrollValue += normalized;
    _scrollValue.x = math::clamp(_scrollValue.x, 0.F, 1.F);
    _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

    return EventResponse::eConsumed;
}
