#include "VBox.hpp"

#include <Engine.Logging/Logger.hpp>
#include "../Style/BoundStyleSheet.hpp"

using namespace ember::engine::reflow;
using namespace ember;

VBox::VBox(mref<sptr<BoundStyleSheet>> style_) :
    Box(_STD move(style_)) {}

VBox::~VBox() = default;

string VBox::getTag() const noexcept {
    return _STD format(R"(VBox <{:#x}>)", reinterpret_cast<u64>(this));
}

static void applyPaddingToOuter(cref<StyleSheet> style_, ref<math::vec2> target_) {
    target_.x -= style_.padding->x;
    target_.y -= style_.padding->y;
    target_.x -= style_.padding->z;
    target_.y -= style_.padding->w;
}

static math::vec2 calcImplicitInnerSize(cref<StyleSheet> style_, cref<math::vec2> space_, cref<math::vec2> limit_) {

    math::vec2 local {};

    /**/

    if (style_.width->type == ReflowUnitType::eRelative) {
        local.x = space_.x * style_.width->value;
    } else if (style_.width->type == ReflowUnitType::eAbsolute) {
        local.x = MIN(style_.width->value, space_.x);
    }

    if (style_.height->type == ReflowUnitType::eRelative) {
        local.y = space_.y * style_.height->value;
    } else if (style_.height->type == ReflowUnitType::eAbsolute) {
        local.y = MIN(style_.height->value, space_.y);
    }

    /**/

    if (style_.minWidth->type != ReflowUnitType::eAuto) {
        if (style_.minWidth->type == ReflowUnitType::eAbsolute) {
            local.x = MAX(local.x, style_.minWidth->value);
        } else if (style_.minWidth->type == ReflowUnitType::eRelative) {
            local.x = space_.x * style_.minWidth->value;
        }
    }

    if (style_.minHeight->type != ReflowUnitType::eAuto) {
        if (style_.minHeight->type == ReflowUnitType::eAbsolute) {
            local.y = MAX(local.y, style_.minHeight->value);
        } else if (style_.minHeight->type == ReflowUnitType::eRelative) {
            local.y = space_.y * style_.minHeight->value;
        }
    }

    /**/

    math::vec2 maxSize { limit_ };

    if (style_.maxWidth->type != ReflowUnitType::eAuto) {
        if (style_.maxWidth->type == ReflowUnitType::eRelative) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value * space_.x);
        } else if (style_.maxWidth->type == ReflowUnitType::eAbsolute) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value);
        }
    }

    if (style_.maxHeight->type != ReflowUnitType::eAuto) {
        if (style_.maxHeight->type == ReflowUnitType::eRelative) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value * space_.y);
        } else if (style_.maxHeight->type == ReflowUnitType::eAbsolute) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value);
        }
    }

    /**/

    return local = compMin<float>(local, maxSize);
}

void clampInnerSize(
    cref<StyleSheet> style_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<math::vec2> target_
) {

    math::vec2 maxSize { limit_ };

    if (style_.maxWidth->type != ReflowUnitType::eAuto) {
        if (style_.maxWidth->type == ReflowUnitType::eRelative) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value * space_.x);
        } else if (style_.maxWidth->type == ReflowUnitType::eAbsolute) {
            maxSize.x = MIN(maxSize.x, style_.maxWidth->value);
        }
    }

    if (style_.maxHeight->type != ReflowUnitType::eAuto) {
        if (style_.maxHeight->type == ReflowUnitType::eRelative) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value * space_.y);
        } else if (style_.maxHeight->type == ReflowUnitType::eAbsolute) {
            maxSize.y = MIN(maxSize.y, style_.maxHeight->value);
        }
    }

    /**/

    applyPaddingToOuter(style_, maxSize);
    target_ = math::compMin<float>(target_, maxSize);
}

void VBox::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width->type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height->type == ReflowUnitType::eAuto };

    /**/

    math::vec2 local { calcImplicitInnerSize(_computedStyle, space_, limit_) };
    applyPaddingToOuter(_computedStyle, local);

    /**/
    math::vec2 innerChildAgg { 0.F };
    math::vec2 innerChildMax { 0.F };
    for (const auto& widget : *children()) {

        constexpr math::vec2 limit { _STD numeric_limits<float>::infinity() };

        if (widget->willChangeLayout(local, styleStack_)) {
            widget->flow(ctx_, local, limit, styleStack_);
        }
        const auto bounding = widget->outerSize();

        if (widget->position() == ReflowPosition::eAbsolute) {
            continue;
        }

        innerChildAgg.x = MAX(innerChildAgg.x, bounding.x);
        innerChildAgg.y += bounding.y;

        innerChildMax = math::compMax<float>(innerChildMax, bounding);
    }

    /**/
    if (_computedStyle.colGap.attr.type != ReflowUnitType::eAuto && children()->size() > 1) {
        if (_computedStyle.colGap.attr.type == ReflowUnitType::eRelative) {
            //local.y += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value * local.y;
            innerChildAgg.y += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value * local.y;
        } else if (_computedStyle.colGap.attr.type == ReflowUnitType::eAbsolute) {
            //local.y += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value;
            innerChildAgg.y += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value;
        }
    }

    /**/
    local.x = MAX(local.x, innerChildMax.x);

    /**/
    if (autoHeight) {
        local.y = MAX(local.y, innerChildAgg.y);
    }

    /**/
    clampInnerSize(_computedStyle, space_, limit_, local);

    /**/
    math::vec2 diff { innerChildAgg - local };

    /**
     * Overflow Behaviour
     */

    /**
     * Overflow Behaviour - Wrapping
     */
    if (diff.y > 0.F && _computedStyle.wrap == ReflowWrap::eWrap) {

        innerChildAgg.x = 0.F;
        innerChildAgg.y = 0.F;

        const auto& bounds { local };
        constexpr math::vec2 limit { _STD numeric_limits<float>::infinity() };

        float forward { 0.F };
        math::vec2 maxForward { 0.F };
        for (const auto& widget : *children()) {

            if (widget->position() == ReflowPosition::eAbsolute) {
                continue;
            }

            widget->flow(ctx_, local, limit, styleStack_);
            const auto bounding { widget->outerSize() };

            if (forward + bounding.y >= bounds.y) {
                innerChildAgg.x += maxForward.x;
                innerChildAgg.y = MAX(innerChildAgg.y, forward);

                if (_computedStyle.rowGap->type != ReflowUnitType::eAuto) {
                    if (_computedStyle.rowGap->type == ReflowUnitType::eRelative) {
                        innerChildAgg.x += local.x * _computedStyle.rowGap->value;
                    } else if (_computedStyle.rowGap->type == ReflowUnitType::eAbsolute) {
                        innerChildAgg.x += _computedStyle.rowGap->value;
                    }
                }

                forward = 0.F;
                maxForward.x = 0.F;
                maxForward.y = 0.F;
            }

            forward += bounding.y;
            maxForward = math::compMax<float>(maxForward, bounding);

            /**/
            if (_computedStyle.colGap.attr.type != ReflowUnitType::eAuto) {
                if (_computedStyle.colGap->type == ReflowUnitType::eRelative) {
                    forward += _computedStyle.colGap->value * bounds.x;
                } else if (_computedStyle.colGap->type == ReflowUnitType::eAbsolute) {
                    forward += _computedStyle.colGap->value;
                }
            }
        }

        /**/
        innerChildAgg.x += maxForward.x;

        /**/
        if (autoWidth) {
            local.x = MAX(local.x, innerChildAgg.x);
        }

        /**/
        clampInnerSize(_computedStyle, space_, limit_, local);

        /**/
        diff = innerChildAgg - local;
    }

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
            IM_CORE_WARN("Applying flex shrinking and wrapping at the same time will break layout (not supported).");
        }
        #endif

        /**/
        for (const auto& pair : rw) {
            const math::vec2 limit {
                _STD numeric_limits<float>::infinity(),
                pair.second.y - perNode.y * pair.first->shrinkFactor()
            };

            pair.first->flow(ctx_, local, limit, styleStack_);
        }

        /**/
        // TODO: Recalculate the innerChildAgg cause shrinking will modify the content size
        // Warning: This is not a actual solution, cause this will compromise the wrapping behaviour
        for (const auto& pair : rw) {
            const auto changed { pair.second - pair.first->outerSize() };
            diff.y -= changed.y;
        }

        #if TRUE
        if (shrinkFrac > 0.F && diff.y > 0.F) {
            IM_CORE_WARNF("Failed to compensate overflow in {}.", getTag());
        }
        #endif
    }

    /**
     * Underflow Behaviour
     */
    if (diff.y < 0.F) { }

    /**
     * Content State
     */
    _innerSize = local;
    styleStack_.popLayer();

    /**
     * State Changes
     */
    clearPending();
    markCaptureState();

    _prevSpace = space_;
    _prevStyleStack.clear();
    styleStack_.compress(_prevStyleStack);
}

void VBox::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (_screenOff != offset_) {
        markCaptureState();
    }

    _screenOff = offset_;

    /**/
    u32 effectNodes { 0ui32 };
    /**/

    math::vec2 aggregate { 0.F };
    for (const auto& widget : *children()) {

        if (widget->position() == ReflowPosition::eAbsolute) {
            continue;
        }

        aggregate += widget->outerSize();
        ++effectNodes;
    }

    const math::vec2 inner { innerSize() };
    math::vec2 left { inner };
    left += aggregate * -1.F;

    /**/
    if (_computedStyle.colGap.attr.type != ReflowUnitType::eAuto && !children()->empty()) {
        left.y -= _computedStyle.colGap.attr.value * (children()->size() - 1);
    }
    /**/

    left = math::compMax<float>(left, math::vec2 { 0.F });

    /**/
    math::vec2 localOffset { offset_ };

    if (!_computedStyle.margin->zero()) {
        localOffset.x += _computedStyle.margin->x;
        localOffset.y += _computedStyle.margin->y;
    }

    if (!_computedStyle.padding->zero()) {
        localOffset.x += _computedStyle.padding->x;
        localOffset.y += _computedStyle.padding->y;
    }

    /**/
    math::vec2 fwd { 0.F };
    math::vec2 ctxDiff { 0.F };

    math::vec2 maxFwd { 0.F };

    u32 fwdIdx { 0ui32 };
    for (const auto& widget : *children()) {

        /**/
        if (widget->position() == ReflowPosition::eAbsolute) {
            widget->shift(ctx_, localOffset);
            continue;
        }
        /**/

        math::vec2 io { 0.F };// Internal Offset
        const auto wouter { widget->outerSize() };

        if (_computedStyle.wrap == ReflowWrap::eWrap) {

            if ((fwd.y + wouter.y) > inner.y) {

                ctxDiff.x += maxFwd.x;

                if (_computedStyle.rowGap->type != ReflowUnitType::eAuto) {
                    if (_computedStyle.rowGap->type == ReflowUnitType::eRelative) {
                        ctxDiff.x += _computedStyle.rowGap->value * inner.x;
                    } else if (_computedStyle.rowGap->type == ReflowUnitType::eAbsolute) {
                        ctxDiff.x += _computedStyle.rowGap->value;
                    }
                }

                fwd.x = 0.F;
                fwd.y = 0.F;

                maxFwd.x = 0.F;
                maxFwd.y = 0.F;
            }

        }

        if (_computedStyle.reflowSpacing == ReflowSpacing::eSpaceEvenly) {

            const math::vec2 ispn { left / (effectNodes + 1) };
            io.y += ispn.y;

        } else if (_computedStyle.reflowSpacing == ReflowSpacing::eSpaceAround) {

            if (fwdIdx == 0/* || fwdIdx == nodeCount */) {
                io.y += left.y / 2.F;
            }

        } else if (_computedStyle.reflowSpacing == ReflowSpacing::eSpaceBetween) {

            // Skip first element for right bound offset when iterating left bound
            if (fwdIdx != 0) {
                const math::vec2 ispn { left / (effectNodes - 1) };
                io.y += ispn.y;
            }

        }

        /**
         * Shift target widget
         */
        math::vec2 offset { localOffset + io + ctxDiff };
        offset.y += fwd.y;

        widget->shift(ctx_, offset);

        /**
         *
         */
        fwd += io;
        fwd += wouter;

        /**/
        maxFwd = math::compMax<float>(maxFwd, wouter);

        /**/
        if (_computedStyle.colGap->type != ReflowUnitType::eAuto) {
            if (_computedStyle.colGap->type == ReflowUnitType::eRelative) {
                fwd.y += _computedStyle.colGap->value * inner.y;
            } else if (_computedStyle.colGap->type == ReflowUnitType::eAbsolute) {
                fwd.y += _computedStyle.colGap->value;
            }
        }

        /**/
        ++fwdIdx;
    }

    /**/
    clearShiftState();
}
