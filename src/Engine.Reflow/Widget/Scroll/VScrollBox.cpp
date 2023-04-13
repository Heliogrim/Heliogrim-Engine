#include "VScrollBox.hpp"

#include <Engine.Logging/Logger.hpp>
#include "../../Layout/Style.hpp"

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

    const math::vec2 off { /*screenOffset()*/ };
    const math::vec2 size { /*contentSize()*/ };

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

void VScrollBox::render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) {

    const auto off = _state.layoutOffset;
    const auto size = _state.layoutSize;

    const math::fExtent2D scissor { size.x, size.y, off.x, off.y };
    cmd_->pushIntersectScissor(scissor);

    /**/

    VerticalPanel::render(state_, cmd_);

    /**/

    if (_scrollTrack) {
        _scrollTrack->render(state_, cmd_);
    }

    if (_scrollThumb) {
        _scrollThumb->render(state_, cmd_);
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
    VerticalPanel::applyLayout(state_, _STD move(ctx_));
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
