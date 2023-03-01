#pragma once

#include <Engine.Reflow/Widget/Menu.hpp>

namespace hg::editor::ui {
    class ContextMenu :
        public engine::reflow::Menu {
    public:
        using this_type = ContextMenu;

    public:
        ContextMenu();

        ~ContextMenu() override;

    public:
        void addMenuAction(mref<sptr<Widget>> menuAction_);

        void removeMenuAction(cref<sptr<Widget>> menuAction_);

    public:
        [[nodiscard]] static sptr<ContextMenu> make(const ptr<Widget> target_, cref<math::ivec2> position_);
    };
}
