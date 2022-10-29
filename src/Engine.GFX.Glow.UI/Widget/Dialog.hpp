#pragma once
#include "Panel.hpp"
#include "Popup.hpp"

namespace ember::engine::gfx::glow::ui {

    class Dialog : public Popup {
    public:
        Dialog();

        ~Dialog() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;
    };
}
