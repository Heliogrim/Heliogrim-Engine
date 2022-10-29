#pragma once
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Overlay : public Widget {
    public:
        using this_type = Overlay;

    public:
        Overlay();

        ~Overlay() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

    public:
        math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_) override;

        void shift(cref<UIContext> ctx_, cref<math::vec2> offset_) override;
    };
}
