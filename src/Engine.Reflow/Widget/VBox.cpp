#include "VBox.hpp"

#include "../Style/BoundStyleSheet.hpp"

#if TRUE
#include <Engine.Common/stdafx.h>
#endif

using namespace ember::engine::reflow;
using namespace ember;

VBox::VBox(mref<sptr<BoundStyleSheet>> style_) :
    Box(_STD move(style_)) {}

VBox::~VBox() = default;

void VBox::flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) {

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

        if (widget->willChangeLayout(local, styleStack_)) {
            widget->flow(ctx_, local, styleStack_);
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
            local.x += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value * local.x;
        } else if (_computedStyle.colGap.attr.type == ReflowUnitType::eAbsolute) {
            local.x += static_cast<float>(children()->size() - 1) * _computedStyle.colGap.attr.value;
        }
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

    /**
     * Overflow Behaviour - Wrapping
     */
    if (diff.y > 0.F && _computedStyle.wrap == ReflowWrap::eWrap) {

        innerChildAgg.x = 0.F;
        innerChildAgg.y = 0.F;

        const auto& bounds { local };

        float forward { 0.F };
        math::vec2 maxForward { 0.F };
        for (const auto& widget : *children()) {

            if (widget->position() == ReflowPosition::eAbsolute) {
                continue;
            }

            widget->flow(ctx_, local, styleStack_);
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

        #if TRUE
        if (diff.y > 0.F) {
            DEBUG_MSG("Failed to compensate overflow in VBox.")
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

    _prevSpace = space_;
    _prevStyleStack.clear();
    styleStack_.compress(_prevStyleStack);
}

void VBox::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

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
                fwd.x += _computedStyle.colGap->value * inner.x;
            } else if (_computedStyle.colGap->type == ReflowUnitType::eAbsolute) {
                fwd.x += _computedStyle.colGap->value;
            }
        }

        /**/
        ++fwdIdx;
    }
}
