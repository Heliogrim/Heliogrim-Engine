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

        innerChildAgg += bounding;
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
    const math::vec2 diff { innerChildAgg - local };

    /**
     * Overflow Behaviour
     */
    if (
        (diff.x > 0.F && self_->_reflowType == ReflowType::eFlexRow) ||
        (diff.y > 0.F && self_->_reflowType == ReflowType::eFlexCol)
    ) {

        float growFrac { 0.F };
        float shrinkFrac { 0.F };

        Vector<ptr<Widget>> rw {};
        for (const auto& widget : self_->_nodes) {

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

    math::vec2 aggregate { 0.F };
    for (const auto& widget : self_->_nodes) {
        //
        aggregate.x += widget->_transform.width;
        aggregate.y += widget->_transform.height;

        //
        aggregate.x += widget->_margin.x;
        aggregate.y += widget->_margin.y;
        aggregate.x += widget->_margin.z;
        aggregate.y += widget->_margin.w;
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
    const auto nodeCount { self_->_nodes.size() };

    for (u32 idx { 0ui32 }; idx < nodeCount; ++idx) {

        const auto& widget { self_->_nodes[idx] };
        math::vec2 io { 0.F };// Internal Offset

        if (self_->_reflowSpacing == ReflowSpacing::eSpaceEvenly) {

            const math::vec2 ispn { left / (self_->_nodes.size() + 1) };

            if (self_->_reflowType == ReflowType::eFlexCol) {
                io.y += ispn.y;
            } else if (self_->_reflowType == ReflowType::eFlexRow) {
                io.x += ispn.x;
            }

        } else if (self_->_reflowSpacing == ReflowSpacing::eSpaceAround) {

            if (idx == 0/* || idx == nodeCount */) {

                if (self_->_reflowType == ReflowType::eFlexCol) {
                    io.y += left.y / 2.F;
                } else if (self_->_reflowType == ReflowType::eFlexRow) {
                    io.x += left.x / 2.F;
                }

            }

        } else if (self_->_reflowSpacing == ReflowSpacing::eSpaceBetween) {

            // Skip first element for right bound offset when iterating left bound
            if (idx != 0) {

                const math::vec2 ispn { left / (nodeCount - 1) };

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
        math::vec2 offset { localOffset + io };

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
    constexpr math::vec4 borderRadius { 0.F };

    /**
     *
     */
    if (borderRadius.zero()) {

        const math::vec2 c0 { tf.offsetX, tf.offsetY };
        const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
        const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
        const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

        cmd_->drawQuad(c0, c1, c2, c3, self_->_background);

    } else {

        constexpr auto deg45rad { glm::radians(45.F) };
        const auto sin45 { glm::sin(deg45rad) };

        const math::vec4 ebr { borderRadius - borderRadius * sin45 };

        /**
         *
         */
        math::vec2 c0 { tf.offsetX + ebr.x, tf.offsetY + ebr.x };
        math::vec2 c1 { (tf.offsetX + tf.width) - ebr.y, tf.offsetY + ebr.y };
        math::vec2 c2 { (tf.offsetX + tf.width) - ebr.z, (tf.offsetY + tf.height) - ebr.z };
        math::vec2 c3 { tf.offsetX + ebr.w, (tf.offsetY + tf.height) - ebr.w };

        cmd_->drawQuad(c0, c1, c2, c3, self_->_background);

        /**
         *
         */
        math::vec2 c00 { tf.offsetX + borderRadius.x, tf.offsetY };
        math::vec2 c01 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY };
        math::vec2 c02 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY + ebr.y };
        math::vec2 c03 { tf.offsetX + borderRadius.x, tf.offsetY + ebr.x };

        cmd_->drawQuad(c00, c01, c02, c03, self_->_background);

        /**
         *
         */
        math::vec2 c10 { tf.offsetX + tf.width, tf.offsetY + borderRadius.y };
        math::vec2 c11 { tf.offsetX + tf.width, tf.offsetY + tf.height - borderRadius.z };
        math::vec2 c12 { tf.offsetX + tf.width - ebr.z, tf.offsetY + tf.height - borderRadius.z };
        math::vec2 c13 { tf.offsetX + tf.width - ebr.y, tf.offsetY + borderRadius.y };

        cmd_->drawQuad(c10, c11, c12, c13, self_->_background);

        /**
         *
         */
        math::vec2 c20 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height };
        math::vec2 c21 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height };
        math::vec2 c22 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height - ebr.w };
        math::vec2 c23 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height - ebr.z };

        cmd_->drawQuad(c20, c21, c22, c23, self_->_background);

        /**
         *
         */
        math::vec2 c30 { tf.offsetX, tf.offsetY + tf.height - borderRadius.w };
        math::vec2 c31 { tf.offsetX, tf.offsetY + borderRadius.x };
        math::vec2 c32 { tf.offsetX + ebr.x, tf.offsetY + borderRadius.x };
        math::vec2 c33 { tf.offsetX + ebr.w, tf.offsetY + tf.height - borderRadius.w };

        cmd_->drawQuad(c30, c31, c32, c33, self_->_background);

        /**
         *
         */
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x },
            borderRadius.x, 270.F, 360.F, self_->_background);
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY + borderRadius.y },
            borderRadius.y, 0.F, 90.F, self_->_background);
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height - borderRadius.z },
            borderRadius.z, 90.F, 180.F, self_->_background);
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height - borderRadius.w },
            borderRadius.w, 180.F, 270.F, self_->_background);
    }

    /**
     *
     */
    for (const auto& widget : self_->_nodes) {
        if (widget->visible()) {
            widget->render(cmd_);
        }
    }

}
