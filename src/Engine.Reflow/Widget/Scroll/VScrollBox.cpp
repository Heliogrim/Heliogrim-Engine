#include "VScrollBox.hpp"

#include "../../Style/BoundStyleSheet.hpp"

#if TRUE
#include <Engine.Common/stdafx.h>
#endif

using namespace ember::engine::reflow;
using namespace ember;

VScrollBox::VScrollBox(mref<sptr<BoundStyleSheet>> style_) :
    VBox(_STD move(style_)) {}

VScrollBox::~VScrollBox() = default;

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

void VScrollBox::render(const ptr<ReflowCommandBuffer> cmd_) {

    const auto cs { contentSize() };
    const math::vec2 off { _screenOff + math::vec2 { _computedStyle.margin.attr.x, _computedStyle.margin.attr.y } };

    const math::fExtent2D scissor { cs.x, cs.y, off.x, off.y };
    cmd_->pushIntersectScissor(scissor);

    /**/

    VBox::render(cmd_);

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

void VScrollBox::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width->type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height->type == ReflowUnitType::eAuto };

    math::vec2 local { 0.F };

    if (_computedStyle.width->type == ReflowUnitType::eRelative) {
        local.x = space_.x * _computedStyle.width->value;
    } else if (_computedStyle.width->type == ReflowUnitType::eAbsolute) {
        local.x = MIN(_computedStyle.width->value, space_.x);
    }

    if (_computedStyle.height->type == ReflowUnitType::eRelative) {
        local.y = space_.y * _computedStyle.height->value;
    } else if (_computedStyle.height->type == ReflowUnitType::eAbsolute) {
        local.y = MIN(_computedStyle.height->value, space_.y);
    }

    /**/
    if (_computedStyle.minWidth->type != ReflowUnitType::eAuto) {
        if (_computedStyle.minWidth->type == ReflowUnitType::eAbsolute) {
            local.x = MAX(local.x, _computedStyle.minWidth->value);
        } else if (_computedStyle.minWidth->type == ReflowUnitType::eRelative) {
            local.x = space_.x * _computedStyle.minWidth->value;
        }
    }

    if (_computedStyle.minHeight->type != ReflowUnitType::eAuto) {
        if (_computedStyle.minHeight->type == ReflowUnitType::eAbsolute) {
            local.y = MAX(local.y, _computedStyle.minHeight->value);
        } else if (_computedStyle.minHeight->type == ReflowUnitType::eRelative) {
            local.y = space_.y * _computedStyle.minHeight->value;
        }
    }

    /**/
    if (not _computedStyle.padding->zero()) {
        local.x = MAX(local.x - (_computedStyle.padding->x + _computedStyle.padding->z), 0.F);
        local.y = MAX(local.y - (_computedStyle.padding->y + _computedStyle.padding->w), 0.F);
    }

    /**/
    math::vec2 innerChildAgg { 0.F };
    math::vec2 innerChildMax { 0.F };
    for (const auto& widget : *children()) {

        widget->flow(ctx_, local, styleStack_);
        const auto bounding = widget->outerSize();

        if (widget->position() == ReflowPosition::eAbsolute) {
            continue;
        }

        innerChildAgg.x = MAX(innerChildAgg.x, bounding.x);
        innerChildAgg.y += bounding.y;

        innerChildMax = math::compMax<float>(innerChildMax, bounding);
    }

    /**/
    local.x = MAX(local.x, innerChildMax.x);

    /**/
    if (autoHeight) {
        local.y = MAX(local.y, innerChildAgg.y);
    }

    /**/
    if (_computedStyle.maxWidth->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxWidth->type == ReflowUnitType::eRelative) {
            local.x = MIN(local.x,
                MAX(_computedStyle.maxWidth->value * space_.x - (_computedStyle.padding->x + _computedStyle.padding->z),
                    0));
        } else if (_computedStyle.maxWidth->type == ReflowUnitType::eAbsolute) {
            local.x = MIN(local.x,
                MAX(_computedStyle.maxWidth->value - (_computedStyle.padding->x + _computedStyle.padding->z), 0));
        }
    }

    if (_computedStyle.maxHeight->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxHeight->type == ReflowUnitType::eRelative) {
            local.y = MIN(local.y,
                MAX(_computedStyle.maxHeight->value * space_.y - (_computedStyle.padding->y + _computedStyle.padding->w)
                    , 0));
        } else if (_computedStyle.maxHeight->type == ReflowUnitType::eAbsolute) {
            local.y = MIN(local.y,
                MAX(_computedStyle.maxHeight->value - (_computedStyle.padding->y + _computedStyle.padding->w), 0));
        }
    }

    /**/
    math::vec2 diff { innerChildAgg - local };

    /**
     * Overflow Behaviour
     */

    // ScrollBox should not have a wrapping behaviour, otherwise we might drop the overflow values

    /**
     * Overflow Behaviour - Flex Shrink
     */
    if (diff.y > 0.F) {

        float shrinkFrac { 0.F };
        Vector<_STD pair<ptr<Widget>, math::vec2>> rw {};

        for (const auto& widget : *children()) {

            if (widget->position() == ReflowPosition::eAbsolute) {
                continue;
            }

            shrinkFrac += widget->shrinkFactor();

            if (widget->shrinkFactor() > 0.F) {
                rw.push_back(_STD make_pair(widget.get(), widget->outerSize()));
            }

        }

        /**/
        math::vec2 perNode { 0.F, diff.y / shrinkFrac };

        #if TRUE
        if (not perNode.zero() && _computedStyle.wrap.attr != ReflowWrap::eNoWrap) {
            DEBUG_NMSG("WARN",
                "Applying flex shrinking and wrapping at the same time will break layout (not supported).")
        }
        #endif

        /**/
        for (const auto& pair : rw) {
            math::vec2 modified { local - perNode * pair.first->shrinkFactor() };
            modified = math::compMax<float>(modified, math::vec2 { 0.F });

            pair.first->flow(ctx_, modified, styleStack_);
        }

        /**/
        // TODO: Recalculate the innerChildAgg cause shrinking will modify the content size
        // Warning: This is not a actual solution, cause this will compromise the wrapping behaviour
        for (const auto& pair : rw) {
            const auto changed { pair.second - pair.first->outerSize() };
            diff.y -= changed.y;
        }
    }

    /**
     * Underflow Behaviour
     */
    if (diff.y < 0.F) { }

    /**
     * Content State
     */
    _overflow = math::compMax<float>(innerChildAgg - local, math::vec2 { 0.F });
    _innerSize = local;
    styleStack_.popLayer();

    clearPending();
    _prevSpace = space_;
    _prevStyleStack.clear();
    styleStack_.compress(_prevStyleStack);

    /**/

    auto scrollSpace { space_ };
    scrollSpace.x = 8.F;

    if (_scrollTrack) {
        _scrollTrack->flow(ctx_, scrollSpace, styleStack_);
    }

    scrollSpace.y = 48.F;

    if (_scrollThumb) {
        _scrollThumb->flow(ctx_, space_, styleStack_);
    }
}

void VScrollBox::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    const math::vec2 scrollOff { _scrollValue * _overflow };
    VBox::shift(ctx_, offset_ - scrollOff);

    _screenOff = offset_;
}

math::vec2 VScrollBox::contentSize() const noexcept {

    math::vec2 size { _innerSize };

    if (not _computedStyle.padding->zero()) {
        size.x += _computedStyle.padding.attr.x;
        size.x += _computedStyle.padding.attr.z;
        size.y += _computedStyle.padding.attr.y;
        size.y += _computedStyle.padding.attr.w;
    }

    return size;
}

EventResponse VScrollBox::onWheel(cref<WheelEvent> event_) {

    if (_children.empty()) {
        _scrollValue = math::vec2 { 0.F };
        return VBox::onWheel(event_);
    }

    constexpr float scrollInvSpeed = 32.F;
    const auto diff { event_._value * (/* _overflow /  */scrollInvSpeed) };
    const auto normalized { diff / _overflow };

    _scrollValue += normalized;
    _scrollValue.x = math::clamp(_scrollValue.x, 0.F, 1.F);
    _scrollValue.y = math::clamp(_scrollValue.y, 0.F, 1.F);

    return EventResponse::eConsumed;
}
