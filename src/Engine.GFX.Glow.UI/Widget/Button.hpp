#pragma once

#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Button : public StatefulWidget {
    public:
        using this_type = Button;
        using Widget::OnClickListener;

    public:
        Button();

        ~Button() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override {

            const auto& tf { _transform };
            constexpr math::vec4 borderRadius { 0.F };

            if (borderRadius.zero()) {

                const math::vec2 c0 { tf.offsetX, tf.offsetY };
                const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
                const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
                const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

                //cmd_->drawQuad(c0, c1, c2, c3, _background);
                cmd_->drawQuad(c0, c1, c2, c3, hovered() ? _raisedColor : _baseColor);

            }

            for (const auto& widget : _nodes) {
                widget->render(cmd_);
            }
        }

        math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_) override {

            if (_nodes.empty()) {
                _minExtent.x = 144.F;
                _minExtent.y = 24.F;
                _extent.x = 0.F;
                _extent.y = 0.F;
            }

            return Reflow::flow(this, ctx_, available_);
        }

        math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) override {
            return math::vec2 { 0.0888888F, 0.03333333F };
        }

    public:
        [[nodiscard]] u64 addOnClick(mref<OnClickListener> listener_);

        //private:
    public:
        color _baseColor { 46.F, 50.F, 58.F, 255.F };
        color _raisedColor { 53.F, 57.F, 65.F, 255.F };

    public:
        bool onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) override;
    };

}
