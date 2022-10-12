#pragma once
#include "Button.hpp"
#include "Menu.hpp"

namespace ember::engine::gfx::glow::ui {

    class MenuButton : public Button {
    public:
        using this_type = MenuButton;

    public:
        MenuButton();

        ~MenuButton() override = default;

    public:
        bool onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) override;

    private:
        sptr<Menu> _menu;
        u64 _rootListenerId;

    public:
        [[nodiscard]] cref<sptr<Menu>> menu() const noexcept;

        void setMenu(cref<sptr<Menu>> menu_);

    };

}
