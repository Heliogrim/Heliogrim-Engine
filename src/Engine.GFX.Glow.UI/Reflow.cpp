#include "Reflow.hpp"

#include "./Widget/Widget.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

math::vec2 Reflow::flow(ptr<Widget> self_, cref<UIContext> ctx_, cref<math::vec2> outerSpace_) {

    const bool autoWidth { self_->_extent.x < 0.F };
    const bool autoHeight { self_->_extent.y < 0.F };

    auto local = self_->_extent * outerSpace_;

    if (autoWidth) {
        local.x = 0.F;
    }
    if (autoHeight) {
        local.y = 0.F;
    }

    /**/
    if (self_->_minExtent.x > 0.F) {
        local.x = MAX(local.x, self_->_minExtent.x);
    }

    if (self_->_minExtent.y > 0.F) {
        local.y = MAX(local.y, self_->_minExtent.y);
    }

    /**/
    if (!self_->_padding.zero()) {
        local.x = MAX(local.x - (self_->_padding.x + self_->_padding.z), 0.F);
        local.y = MAX(local.y - (self_->_padding.y + self_->_padding.w), 0.F);
    }

    /**/
    math::vec2 innerChildAgg { 0.F };
    math::vec2 innerChildMax { 0.F };
    for (const auto& widget : self_->_nodes) {

        const auto bounding = widget->flow(ctx_, local);

        if (widget->_reflowPosition == ReflowPosition::eAbsolute) {
            continue;
        }

        if (self_->_reflowType == ReflowType::eFlexRow) {
            innerChildAgg.x += bounding.x;
            innerChildAgg.y = MAX(innerChildAgg.y, bounding.y);

        } else if (self_->_reflowType == ReflowType::eFlexCol) {
            innerChildAgg.x = MAX(innerChildAgg.x, bounding.x);
            innerChildAgg.y += bounding.y;
        }

        innerChildMax = math::compMax<float>(innerChildMax, bounding);
    }

    /**/
    if (self_->_reflowType == ReflowType::eFlexCol) {
        local.x = MAX(local.x, innerChildMax.x);
    } else if (self_->_reflowType == ReflowType::eFlexRow) {
        local.y = MAX(local.y, innerChildMax.y);
    }

    /**/
    if (autoWidth && self_->_reflowType == ReflowType::eFlexRow) {
        local.x = MAX(local.x, innerChildAgg.x);
    }

    if (autoHeight && self_->_reflowType == ReflowType::eFlexCol) {
        local.y = MAX(local.y, innerChildAgg.y);
    }

    /**/
    math::vec2 diff { innerChildAgg - local };

    /**
     * Overflow Behaviour
     */

    /**
     * Overflow Behaviour - Wrapping
     */
    if (
        (
            (diff.x > 0.F && self_->_reflowType == ReflowType::eFlexRow) ||
            (diff.y > 0.F && self_->_reflowType == ReflowType::eFlexCol)
        ) && (
            self_->_reflowWrapping == ReflowWrapping::eWrap
        )
    ) {

        innerChildAgg.x = 0.F;
        innerChildAgg.y = 0.F;

        const auto& bounds { local };

        float forward { 0.F };
        math::vec2 maxForward { 0.F };
        for (const auto& widget : self_->_nodes) {

            if (widget->_reflowPosition == ReflowPosition::eAbsolute) {
                continue;
            }

            const auto bounding = widget->flow(ctx_, local);

            if (self_->_reflowType == ReflowType::eFlexRow) {

                if (forward + bounding.x >= bounds.x) {
                    innerChildAgg.x = MAX(innerChildAgg.x, forward);
                    innerChildAgg.y += maxForward.y;

                    innerChildAgg.y += self_->_reflowGapping.y;

                    forward = 0.F;
                    maxForward.y = 0.F;
                    maxForward.x = 0.F;
                }

                forward += bounding.x;
                maxForward = math::compMax<float>(maxForward, bounding);

            } else if (self_->_reflowType == ReflowType::eFlexCol) {

                if (forward + bounding.y >= bounds.y) {
                    innerChildAgg.x += maxForward.x;
                    innerChildAgg.y = MAX(innerChildAgg.y, forward);

                    innerChildAgg.x += self_->_reflowGapping.x;

                    forward = 0.F;
                    maxForward.x = 0.F;
                    maxForward.y = 0.F;
                }

                forward += bounding.y;
                maxForward = math::compMax<float>(maxForward, bounding);

            }

        }

        /**/
        if (self_->_reflowType == ReflowType::eFlexRow) {
            innerChildAgg.y += maxForward.y;
        } else if (self_->_reflowType == ReflowType::eFlexCol) {
            innerChildAgg.x += maxForward.x;
        }

        /**/
        if (autoWidth && self_->_reflowType == ReflowType::eFlexCol) {
            local.x = MAX(local.x, innerChildAgg.x);
        }

        if (autoHeight && self_->_reflowType == ReflowType::eFlexRow) {
            local.y = MAX(local.y, innerChildAgg.y);
        }

        /**/
        diff = innerChildAgg - local;
    }

    /**
     * Overflow Behaviour - Flex Shrink
     */
    if (
        (diff.x > 0.F && self_->_reflowType == ReflowType::eFlexRow) ||
        (diff.y > 0.F && self_->_reflowType == ReflowType::eFlexCol)
    ) {

        float growFrac { 0.F };
        float shrinkFrac { 0.F };

        Vector<ptr<Widget>> rw {};
        for (const auto& widget : self_->_nodes) {

            if (widget->_reflowPosition == ReflowPosition::eAbsolute) {
                continue;
            }

            growFrac += widget->_reflowGrow;
            shrinkFrac += widget->_reflowShrink;

            if (widget->_reflowShrink > 0.F) {
                rw.push_back(widget.get());
            }

        }

        /**/
        math::vec2 perNode { 0.F };

        if (diff.x > 0.F && self_->_reflowType == ReflowType::eFlexRow) {
            perNode.x = diff.x / shrinkFrac;
        }

        if (diff.y > 0.F && self_->_reflowType == ReflowType::eFlexCol) {
            perNode.y = diff.y / shrinkFrac;
        }

        /**/
        for (const auto& widget : rw) {
            math::vec2 modified { local - perNode * widget->_reflowShrink };
            modified = math::compMax<float>(modified, math::vec2 { 0.F });

            widget->flow(ctx_, modified);
        }
    }

    /**
     * Underflow Behaviour
     */
    if (
        (diff.x < 0.F && self_->reflowType() == ReflowType::eFlexRow) ||
        (diff.y < 0.F && self_->reflowType() == ReflowType::eGrid)
    ) { }

    /**
     * Overflow State
     */
    self_->_overflowSize = math::compMax<float>(innerChildAgg - local, math::vec2 { 0.F });

    /**/
    if (!self_->_padding.zero()) {
        local.x += (self_->_padding.x + self_->_padding.z);
        local.y += (self_->_padding.y + self_->_padding.w);
    }

    /**/
    self_->_transform.width = local.x;
    self_->_transform.height = local.y;

    /* Add local element margin to report correct outer space requirement */
    local.x += (self_->_margin.x + self_->_margin.z);
    local.y += (self_->_margin.y + self_->_margin.w);

    return local;
}

math::vec2 Reflow::shift(ptr<Widget> self_, cref<UIContext> ctx_, cref<math::vec2> outerOffset_) {

    /**
     *
     */
    self_->_transform.offsetX = outerOffset_.x;
    self_->_transform.offsetY = outerOffset_.y;

    /**
     *
     */
    assert(self_->_reflowType == ReflowType::eFlexCol || self_->_reflowType == ReflowType::eFlexRow);

    /**/
    u32 effectNodes { 0ui32 };
    /**/

    math::vec2 aggregate { 0.F };
    for (const auto& widget : self_->_nodes) {

        //
        if (widget->_reflowPosition == ReflowPosition::eAbsolute) {
            continue;
        }

        //
        aggregate.x += widget->_transform.width;
        aggregate.y += widget->_transform.height;

        //
        aggregate.x += widget->_margin.x;
        aggregate.y += widget->_margin.y;
        aggregate.x += widget->_margin.z;
        aggregate.y += widget->_margin.w;

        //
        ++effectNodes;
    }

    math::vec2 left {
        self_->_transform.width - (self_->_padding.x + self_->_padding.z),
        self_->_transform.height - (self_->_padding.y + self_->_padding.w)
    };
    left += aggregate * -1.F;

    left = math::compMax<float>(left, math::vec2 { 0.F });

    /**/
    math::vec2 localOffset { outerOffset_ };

    if (!self_->_padding.zero()) {
        localOffset.x += self_->_padding.x;
        localOffset.y += self_->_padding.y;
    }

    /**/
    math::vec2 fwd { 0.F };
    math::vec2 ctxDiff { 0.F };
    const auto nodeCount { self_->_nodes.size() };

    math::vec2 maxFwd { 0.F };

    u32 fwdIdx { 0ui32 };
    for (const auto& widget : self_->_nodes) {

        /**/
        if (widget->_reflowPosition == ReflowPosition::eAbsolute) {
            widget->shift(ctx_, localOffset);
            continue;
        }
        /**/

        math::vec2 io { 0.F };// Internal Offset

        if (self_->_reflowWrapping == ReflowWrapping::eWrap) {

            if (self_->_reflowType == ReflowType::eFlexRow && (fwd.x + widget->_transform.width + widget->_margin.x +
                widget->_margin.z) > self_->_transform.width) {

                ctxDiff.y += maxFwd.y;
                ctxDiff.y += self_->_reflowGapping.y;

                fwd.x = 0.F;
                fwd.y = 0.F;

                maxFwd.x = 0.F;
                maxFwd.y = 0.F;

            } else if (self_->_reflowType == ReflowType::eFlexCol && (fwd.y + widget->_transform.height + widget->
                _margin.y + widget->_margin.w) > self_->_transform.height) {

                ctxDiff.x += maxFwd.x;
                ctxDiff.y += self_->_reflowGapping.x;

                fwd.x = 0.F;
                fwd.y = 0.F;

                maxFwd.x = 0.F;
                maxFwd.y = 0.F;

            }

        }

        if (self_->_reflowSpacing == ReflowSpacing::eSpaceEvenly) {

            const math::vec2 ispn { left / (effectNodes + 1) };

            if (self_->_reflowType == ReflowType::eFlexCol) {
                io.y += ispn.y;
            } else if (self_->_reflowType == ReflowType::eFlexRow) {
                io.x += ispn.x;
            }

        } else if (self_->_reflowSpacing == ReflowSpacing::eSpaceAround) {

            if (fwdIdx == 0/* || fwdIdx == nodeCount */) {

                if (self_->_reflowType == ReflowType::eFlexCol) {
                    io.y += left.y / 2.F;
                } else if (self_->_reflowType == ReflowType::eFlexRow) {
                    io.x += left.x / 2.F;
                }

            }

        } else if (self_->_reflowSpacing == ReflowSpacing::eSpaceBetween) {

            // Skip first element for right bound offset when iterating left bound
            if (fwdIdx != 0) {

                const math::vec2 ispn { left / (effectNodes - 1) };

                if (self_->_reflowType == ReflowType::eFlexCol) {
                    io.y += ispn.y;
                } else if (self_->_reflowType == ReflowType::eFlexRow) {
                    io.x += ispn.x;
                }

            }

        }

        /**
         * Apply relevant margin before shift target widget
         */
        io.x += widget->_margin.x;
        io.y += widget->_margin.y;

        /**
         * Shift target widget
         */
        math::vec2 offset { localOffset + io + ctxDiff };

        if (self_->_reflowType == ReflowType::eFlexCol) {
            offset.y += fwd.y;
        } else if (self_->_reflowType == ReflowType::eFlexRow) {
            offset.x += fwd.x;
        }

        widget->shift(ctx_, offset);

        /**
         * Apply inverse relevant margin after shift target widget
         */
        io.x += widget->_margin.z;
        io.y += widget->_margin.w;

        /**
         *
         */
        fwd += io;

        fwd.x += widget->_transform.width;
        fwd.y += widget->_transform.height;

        /**/
        maxFwd.x = MAX(maxFwd.x, widget->_transform.width + widget->_margin.x + widget->_margin.z);
        maxFwd.y = MAX(maxFwd.y, widget->_transform.height + widget->_margin.y + widget->_margin.w);

        /**/
        ++fwdIdx;
    }

    return outerOffset_;
}

void Reflow::renderContainer(ptr<Widget> self_, const ptr<UICommandBuffer> cmd_) {

    /**
     *
     */
    const auto& tf { self_->_transform };

    /**
     *
     */
    const math::vec4& borderRadius { self_->_borderRadius };

    /**
     *
     */
    if (borderRadius.zero()) {

        const math::vec2 c0 { tf.offsetX, tf.offsetY };
        const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
        const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
        const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

        cmd_->drawQuad(c0, c1, c2, c3, self_->statedColor());

    } else {

        /**
         *
         */
        math::vec2 c0 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x };
        math::vec2 c1 { (tf.offsetX + tf.width) - borderRadius.y, tf.offsetY + borderRadius.y };
        math::vec2 c2 { (tf.offsetX + tf.width) - borderRadius.z, (tf.offsetY + tf.height) - borderRadius.z };
        math::vec2 c3 { tf.offsetX + borderRadius.w, (tf.offsetY + tf.height) - borderRadius.w };

        cmd_->drawQuad(c0, c1, c2, c3, self_->statedColor());

        /**
         *
         */
        math::vec2 c00 { tf.offsetX + borderRadius.x, tf.offsetY };
        math::vec2 c01 { (tf.offsetX + tf.width) - borderRadius.y, tf.offsetY };
        math::vec2 c02 { (tf.offsetX + tf.width) - borderRadius.y, tf.offsetY + borderRadius.y };
        math::vec2 c03 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x };

        cmd_->drawQuad(c00, c01, c02, c03, self_->statedColor());

        /**
         *
         */
        math::vec2 c10 { tf.offsetX + tf.width, tf.offsetY + borderRadius.y };
        math::vec2 c11 { tf.offsetX + tf.width, (tf.offsetY + tf.height) - borderRadius.z };
        math::vec2 c12 { (tf.offsetX + tf.width) - borderRadius.z, (tf.offsetY + tf.height) - borderRadius.z };
        math::vec2 c13 { (tf.offsetX + tf.width) - borderRadius.y, tf.offsetY + borderRadius.y };

        cmd_->drawQuad(c10, c11, c12, c13, self_->statedColor());

        /**
         *
         */
        math::vec2 c20 { (tf.offsetX + tf.width) - borderRadius.z, tf.offsetY + tf.height };
        math::vec2 c21 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height };
        math::vec2 c22 { tf.offsetX + borderRadius.w, (tf.offsetY + tf.height) - borderRadius.w };
        math::vec2 c23 { (tf.offsetX + tf.width) - borderRadius.z, (tf.offsetY + tf.height) - borderRadius.z };

        cmd_->drawQuad(c20, c21, c22, c23, self_->statedColor());

        /**
         *
         */
        math::vec2 c30 { tf.offsetX, (tf.offsetY + tf.height) - borderRadius.w };
        math::vec2 c31 { tf.offsetX, tf.offsetY + borderRadius.x };
        math::vec2 c32 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x };
        math::vec2 c33 { tf.offsetX + borderRadius.w, (tf.offsetY + tf.height) - borderRadius.w };

        cmd_->drawQuad(c30, c31, c32, c33, self_->statedColor());

        /**
         *
         */
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x },
            borderRadius.x, glm::radians(180.F), glm::radians(270.F), self_->statedColor());
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY + borderRadius.y },
            borderRadius.y, glm::radians(270.F), glm::radians(360.F), self_->statedColor());
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height - borderRadius.z },
            borderRadius.z, glm::radians(0.F), glm::radians(90.F), self_->statedColor());
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height - borderRadius.w },
            borderRadius.w, glm::radians(90.F), glm::radians(180.F), self_->statedColor());
    }

    /**
     *
     */
    for (const auto& widget : self_->_nodes) {
        // Warning: Temporary Solution
        const math::vec2 center {
            widget->_transform.offsetX + widget->_transform.width * 0.5F,
            widget->_transform.offsetY + widget->_transform.height * 0.5F
        };

        //if (widget->visible() && not cmd_->scissorCull(center, _STD sqrtf(widget->_transform.width^2 + widget->_transform.height^2))) {
        if (widget->visible() && not cmd_->scissorCull(center,
            (widget->_transform.width + widget->_transform.height) * 0.75F)) {
            widget->render(cmd_);
        }
    }

}
