#pragma once
#include "ReflowContainer.hpp"
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Panel : public ReflowContainer {
    public:
        Panel();

        ~Panel() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

    };

}
