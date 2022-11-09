#pragma once
#include <Engine.Reflow/Widget/Panel.hpp>

namespace ember::editor::ui {

    class SceneHierarchyPanel :
        public engine::reflow::Panel {
    protected:
        SceneHierarchyPanel();

    public:
        static sptr<Panel> make();
    };

}
