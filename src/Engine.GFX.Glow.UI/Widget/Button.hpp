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
            Reflow::renderContainer(this, cmd_);
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
        color _raisedColor { 53.F, 57.F, 65.F, 255.F };

    public:
        [[nodiscard]] engine::color statedColor() const noexcept override;

    public:
        bool onMouseMotionEvent(cref<math::ivec2> pointer_, cref<math::ivec2> delta_, u32 button_,
            u32 modifier_) override;

        bool onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) override;
    };

}
