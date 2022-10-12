#pragma once
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class ReflowContainer :
        public Widget {
    public:
        ReflowContainer();

        ~ReflowContainer() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

        math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) override;
    };

}
