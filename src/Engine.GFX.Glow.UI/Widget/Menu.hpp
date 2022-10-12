#pragma once
#include "Popup.hpp"

namespace ember::engine::gfx::glow::ui {

    class Menu : public Popup {
    public:
        using this_type = Menu;

    public:
        Menu();

        ~Menu() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

    public:
        bool onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) override;
    };

}
