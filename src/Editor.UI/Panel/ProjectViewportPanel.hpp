#pragma once
#include <Engine.GFX.Glow.UI/Widget/Panel.hpp>

namespace ember::editor::ui {

    class ProjectViewportPanel :
        public engine::gfx::glow::ui::Panel {
    protected:
        ProjectViewportPanel();

    public:
        static sptr<Panel> make();
    };

}
