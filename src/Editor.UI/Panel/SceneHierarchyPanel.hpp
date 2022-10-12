#pragma once
#include <Engine.GFX.Glow.UI/Widget/Panel.hpp>

namespace ember::editor::ui {

    class SceneHierarchyPanel :
        public engine::gfx::glow::ui::Panel {
    protected:
        SceneHierarchyPanel();

    public:
        static sptr<Panel> make();
    };

}
