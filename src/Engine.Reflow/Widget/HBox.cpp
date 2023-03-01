#include "HBox.hpp"

#include "../Style/BoundStyleSheet.hpp"
#include <Engine.Logging/Logger.hpp>

using namespace hg::engine::reflow;
using namespace hg;

HBox::HBox(mref<sptr<BoundStyleSheet>> style_) :
    Box(_STD move(style_)) {}

HBox::~HBox() = default;

string HBox::getTag() const noexcept {
    return _STD format(R"(HBox <{:#x}>)", reinterpret_cast<u64>(this));
}

static void applyPaddingToOuter(cref<StyleSheet> style_, ref<math::vec2> target_) {
    target_.x -= style_.padding->x;
    target_.y -= style_.padding->y;
    target_.x -= style_.padding->z;
    target_.y -= style_.padding->w;
}

void HBox::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {

    styleStack_.pushLayer();
    _computedStyle = _style->compute(shared_from_this(), styleStack_);

    const bool autoWidth { _computedStyle.width.attr.type == ReflowUnitType::eAuto };
    const bool autoHeight { _computedStyle.height.attr.type == ReflowUnitType::eAuto };

    /**/

    math::vec2 local { 0.F };

    if (_computedStyle.width.attr.type == ReflowUnitType::eRelative) {
        local.x = space_.x * _computedStyle.width.attr.value;
    } else if (_computedStyle.width.attr.type == ReflowUnitType::eAbsolute) {
        local.x = MIN(_computedStyle.width.attr.value, space_.x);
    }

    if (_computedStyle.height.attr.type == ReflowUnitType::eRelative) {
        local.y = space_.y * _computedStyle.height.attr.value;
    } else if (_computedStyle.height.attr.type == ReflowUnitType::eAbsolute) {
        local.y = MIN(_computedStyle.height.attr.value, space_.y);
    }

    /**/
    if (_computedStyle.minWidth.attr.type != ReflowUnitType::eAuto) {
        if (_computedStyle.minWidth.attr.type == ReflowUnitType::eAbsolute) {
            local.x = MAX(local.x, _computedStyle.minWidth.attr.value);
        } else if (_computedStyle.minWidth.attr.type == ReflowUnitType::eRelative) {
            local.x = space_.x * _computedStyle.minWidth.attr.value;
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

    math::vec2 maxSize { limit_ };
    applyPaddingToOuter(_computedStyle, maxSize);

    if (_computedStyle.maxWidth->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxWidth->type == ReflowUnitType::eRelative) {
            maxSize.x = MIN(
                maxSize.x,
                MAX(_computedStyle.maxWidth->value * space_.x - (_computedStyle.padding->x + _computedStyle.padding->z),
                    0)
            );
        } else if (_computedStyle.maxWidth->type == ReflowUnitType::eAbsolute) {
            maxSize.x = MIN(
                maxSize.x,
                MAX(_computedStyle.maxWidth->value - (_computedStyle.padding->x + _computedStyle.padding->z), 0)
            );
        }
    }

    if (_computedStyle.maxHeight->type != ReflowUnitType::eAuto) {
        if (_computedStyle.maxHeight->type == ReflowUnitType::eRelative) {
            maxSize.y = MIN(
                maxSize.y,
                MAX(_computedStyle.maxHeight->value * space_.y - (_computedStyle.padding->y + _computedStyle.padding->w)
                    , 0)
            );
        } else if (_computedStyle.maxHeight->type == ReflowUnitType::eAbsolute) {
            maxSize.y = MIN(
                maxSize.y,
                MAX(_computedStyle.maxHeight->value - (_computedStyle.padding->y + _computedStyle.padding->w), 0)
            );
        }
    }

    /**/
    if (not _computedStyle.padding->zero()) {
        local.x = MAX(local.x - (_computedStyle.padding->x + _computedStyle.padding->z), 0.F);
        local.y = MAX(local.y - (_computedStyle.padding->y + _computedStyle.padding->w), 0.F);
    }

    /**/
    local = math::compMin<float>(local, maxSize);

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

        innerChildAgg.x += bounding.x;
        innerChildAgg.y = MAX(innerChildAgg.y, bounding.y);

        innerChildMax = math::compMax<float>(innerChildMax, bounding);
    }

    /**/
    if (_computedStyle.rowGap.attr.type != ReflowUnitType::eAuto && children()->size() > 1) {
        if (_computedStyle.rowGap.attr.type == ReflowUnitType::eRelative) {
            //local.x += static_cast<float>(children()->size() - 1) * _computedStyle.rowGap.attr.value * local.x;
            innerChildAgg.x += static_cast<float>(children()->size() - 1) * _computedStyle.rowGap.attr.value * local.x;
        } else if (_computedStyle.rowGap.attr.type == ReflowUnitType::eAbsolute) {
            //local.x += static_cast<float>(children()->size() - 1) * _computedStyle.rowGap.attr.value;
            innerChildAgg.x += static_cast<float>(children()->size() - 1) * _computedStyle.rowGap.attr.value;
        }
    }

    /**/
    local.y = MAX(local.y, innerChildMax.y);

    /**/
    if (autoWidth) {
        local.x = MAX(local.x, innerChildAgg.x);
    }

    /**/
    local = math::compMin<float>(local, maxSize);

    /**/
    math::vec2 diff { innerChildAgg - local };

    /**
     * Overflow Behaviour
     */

    /**
     * Overflow Behaviour - Wrapping
     */
    if (diff.x > 0.F && _computedStyle.wrap == ReflowWrap::eWrap) {

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

            if (forward + bounding.x >= bounds.x) {
                innerChildAgg.x = MAX(innerChildAgg.x, forward);
                innerChildAgg.y += maxForward.y;

                if (_computedStyle.colGap->type != ReflowUnitType::eAuto) {
                    if (_computedStyle.colGap->type == ReflowUnitType::eRelative) {
                        innerChildAgg.y += local.y * _computedStyle.colGap->value;
                    } else if (_computedStyle.colGap->type == ReflowUnitType::eAbsolute) {
                        innerChildAgg.y += _computedStyle.colGap->value;
                    }
                }

                forward = 0.F;
                maxForward.x = 0.F;
                maxForward.y = 0.F;
            }

            forward += bounding.x;
            maxForward = math::compMax<float>(maxForward, bounding);

            /**/
            if (_computedStyle.rowGap.attr.type != ReflowUnitType::eAuto) {
                if (_computedStyle.rowGap->type == ReflowUnitType::eRelative) {
                    forward += _computedStyle.rowGap->value * bounds.x;
                } else if (_computedStyle.rowGap->type == ReflowUnitType::eAbsolute) {
                    forward += _computedStyle.rowGap->value;
                }
            }
        }

        /**/
        innerChildAgg.y += maxForward.y;

        /**/
        if (autoHeight) {
            local.y = MAX(local.y, innerChildAgg.y);
        }

        /**/
        local = math::compMin<float>(local, maxSize);

        /**/
        diff = innerChildAgg - local;
    }

    /**
     * Overflow Behaviour - Flex Shrink
     */
    if (diff.x > 0.F) {

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
        math::vec2 perNode { diff.x / shrinkFrac, 0.F };

        #if TRUE
        if (not perNode.zero() && _computedStyle.wrap.attr != ReflowWrap::eNoWrap) {
            IM_CORE_WARN("Applying flex shrinking and wrapping at the same time will break layout (not supported).");
        }
        #endif

        /**/
        for (const auto& pair : rw) {
            const math::vec2 limit {
                pair.second.x - perNode.x * pair.first->shrinkFactor(),
                _STD numeric_limits<float>::infinity()
            };

            pair.first->flow(ctx_, local, limit, styleStack_);
        }

        /**/
        // TODO: Recalculate the innerChildAgg cause shrinking will modify the content size
        // Warning: This is not a actual solution, cause this will compromise the wrapping behaviour
        for (const auto& pair : rw) {
            const auto changed { pair.second - pair.first->outerSize() };
            diff.x -= changed.x;
        }

        #if TRUE
        if (diff.x > 0.F && shrinkFrac > 0.F) {
            //DEBUG_MSG("Failed to compensate overflow in HBox.")
        }
        #endif
    }

    /**
     * Underflow Behaviour
     */
    if (diff.x < 0.F) { }

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

void HBox::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

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
    if (_computedStyle.rowGap.attr.type != ReflowUnitType::eAuto && !children()->empty()) {
        left.x -= _computedStyle.rowGap.attr.value * (children()->size() - 1);
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

            if ((fwd.x + wouter.x) > inner.x) {

                ctxDiff.y += maxFwd.y;

                if (_computedStyle.colGap->type != ReflowUnitType::eAuto) {
                    if (_computedStyle.colGap->type == ReflowUnitType::eRelative) {
                        ctxDiff.y += _computedStyle.colGap->value * inner.y;
                    } else if (_computedStyle.colGap->type == ReflowUnitType::eAbsolute) {
                        ctxDiff.y += _computedStyle.colGap->value;
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
            io.x += ispn.x;

        } else if (_computedStyle.reflowSpacing == ReflowSpacing::eSpaceAround) {

            if (fwdIdx == 0/* || fwdIdx == nodeCount */) {
                io.x += left.x / 2.F;
            }

        } else if (_computedStyle.reflowSpacing == ReflowSpacing::eSpaceBetween) {

            // Skip first element for right bound offset when iterating left bound
            if (fwdIdx != 0) {
                const math::vec2 ispn { left / (effectNodes - 1) };
                io.x += ispn.x;
            }

        }

        /**
         * Shift target widget
         */
        math::vec2 offset { localOffset + io + ctxDiff };
        offset.x += fwd.x;

        widget->shift(ctx_, offset);

        /**
         *
         */
        fwd += io;
        fwd += wouter;

        /**/
        maxFwd = math::compMax<float>(maxFwd, wouter);

        /**/
        if (_computedStyle.rowGap.attr.type != ReflowUnitType::eAuto) {
            if (_computedStyle.rowGap->type == ReflowUnitType::eRelative) {
                fwd.x += _computedStyle.rowGap->value * inner.x;
            } else if (_computedStyle.rowGap->type == ReflowUnitType::eAbsolute) {
                fwd.x += _computedStyle.rowGap->value;
            }
        }

        /**/
        ++fwdIdx;
    }

    /**/
    clearShiftState();
}
