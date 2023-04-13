#pragma once
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
    class ProjectViewportPanel :
        public engine::reflow::VerticalPanel {
    protected:
        ProjectViewportPanel();

    public:
        static sptr<ProjectViewportPanel> make();
    };
}
