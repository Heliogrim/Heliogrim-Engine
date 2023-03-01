#pragma once
#include <Engine.Reflow/Widget/Panel.hpp>

namespace hg::editor::ui {
    class ProjectViewportPanel :
        public engine::reflow::Panel {
    protected:
        ProjectViewportPanel();

    public:
        static sptr<Panel> make();
    };
}
