#pragma once
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Image : public Widget {
    public:
        Image();

    public:
        void render(const ptr<UICommandBuffer> cmd_) override {

            const color cl { 255.F, 255.F, 255.F, 255.F };

            const auto& tf { _transform };
            constexpr math::vec4 borderRadius { 0.F };

            if (borderRadius.zero()) {

                const math::vec2 c0 { tf.offsetX, tf.offsetY };
                const math::vec2 c1 { tf.offsetX + tf.width, tf.offsetY };
                const math::vec2 c2 { tf.offsetX + tf.width, tf.offsetY + tf.height };
                const math::vec2 c3 { tf.offsetX, tf.offsetY + tf.height };

                cmd_->drawImage(
                    c0, _uvs[0],
                    c1, _uvs[1],
                    c2, _uvs[2],
                    c3, _uvs[3],
                    _image,
                    cl
                );

            }

            for (const auto& widget : _nodes) {
                widget->render(cmd_);
            }

        }

        math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) override {
            return math::vec2 { 0.375F, 0.4444444F } + reflow_;
        }

        //private:
    public:
        sptr<Texture> _image;
        _STD array<math::vec2, 4> _uvs;
    };

}
