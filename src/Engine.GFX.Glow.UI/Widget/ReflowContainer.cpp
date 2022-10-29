#include "ReflowContainer.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

ReflowContainer::ReflowContainer() :
    Widget() {

    _reflowType = ReflowType::eFlexRow;
    _reflowSpacing = ReflowSpacing::eSpaceAround;

    _transform.offsetX = 0.F;
    _transform.offsetY = 0.F;
    _transform.width = 1.F;
    _transform.height = 1.F;

    //_color = color { 0.2F, 0.2F, 0.2F, 1.0F };

    auto rnd = static_cast<float>(rand());
    const auto rndMax = static_cast<float>(RAND_MAX);
    rnd /= rndMax;
    rnd = 0.2F + 0.8F * rnd;

    _color = color(rnd * 255.F, rnd * 255.F, rnd * 255.F, 255.F);
}

void ReflowContainer::render(const ptr<UICommandBuffer> cmd_) {

    if constexpr (true) {
        return Reflow::renderContainer(this, cmd_);
    }

    /**
     *
     */
    const auto& tf { _transform };

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

        cmd_->drawQuad(c0, c1, c2, c3, _color);

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

        cmd_->drawQuad(c0, c1, c2, c3, _color);

        /**
         *
         */
        math::vec2 c00 { tf.offsetX + borderRadius.x, tf.offsetY };
        math::vec2 c01 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY };
        math::vec2 c02 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY + ebr.y };
        math::vec2 c03 { tf.offsetX + borderRadius.x, tf.offsetY + ebr.x };

        cmd_->drawQuad(c00, c01, c02, c03, _color);

        /**
         *
         */
        math::vec2 c10 { tf.offsetX + tf.width, tf.offsetY + borderRadius.y };
        math::vec2 c11 { tf.offsetX + tf.width, tf.offsetY + tf.height - borderRadius.z };
        math::vec2 c12 { tf.offsetX + tf.width - ebr.z, tf.offsetY + tf.height - borderRadius.z };
        math::vec2 c13 { tf.offsetX + tf.width - ebr.y, tf.offsetY + borderRadius.y };

        cmd_->drawQuad(c10, c11, c12, c13, _color);

        /**
         *
         */
        math::vec2 c20 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height };
        math::vec2 c21 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height };
        math::vec2 c22 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height - ebr.w };
        math::vec2 c23 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height - ebr.z };

        cmd_->drawQuad(c20, c21, c22, c23, _color);

        /**
         *
         */
        math::vec2 c30 { tf.offsetX, tf.offsetY + tf.height - borderRadius.w };
        math::vec2 c31 { tf.offsetX, tf.offsetY + borderRadius.x };
        math::vec2 c32 { tf.offsetX + ebr.x, tf.offsetY + borderRadius.x };
        math::vec2 c33 { tf.offsetX + ebr.w, tf.offsetY + tf.height - borderRadius.w };

        cmd_->drawQuad(c30, c31, c32, c33, _color);

        /**
         *
         */
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.x, tf.offsetY + borderRadius.x },
            borderRadius.x, 270.F, 360.F, _color);
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.y, tf.offsetY + borderRadius.y },
            borderRadius.y, 0.F, 90.F, _color);
        cmd_->drawArc(math::vec2 { tf.offsetX + tf.width - borderRadius.z, tf.offsetY + tf.height - borderRadius.z },
            borderRadius.z, 90.F, 180.F, _color);
        cmd_->drawArc(math::vec2 { tf.offsetX + borderRadius.w, tf.offsetY + tf.height - borderRadius.w },
            borderRadius.w, 180.F, 270.F, _color);
    }

    /**
     *
     */
    for (const auto& widget : _nodes) {
        if (widget->visible()) {
            widget->render(cmd_);
        }
    }
}

math::vec2 ReflowContainer::reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) {

    if (reflow_.zero()) {
        return reflow_;
    }

    __debugbreak();
    return reflow_;

    UIContext ctx {};

    // Warning: Only support grow at the current time
    math::vec2 consumed { 0.F };
    if (reflow_.x > 0.F || reflow_.y > 0.F) {

        const auto pre = flow(ctx, available_);

        consumed = pre + reflow_;

        if (reflow_.x > 0.F) {
            consumed.y = MAX(available_.y, consumed.y);
        } else {
            consumed.x = MAX(available_.x, consumed.x);
        }

        [[maybe_unused]] const auto con = flow(ctx, consumed);
    }

    /**
     *
     */
    _transform.width = consumed.x;
    _transform.height = consumed.y;

    return consumed;
}