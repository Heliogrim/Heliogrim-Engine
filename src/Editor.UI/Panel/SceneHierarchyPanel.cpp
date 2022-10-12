#include "SceneHierarchyPanel.hpp"

using namespace ember::engine::gfx::glow::ui;
using namespace ember::editor::ui;
using namespace ember;

SceneHierarchyPanel::SceneHierarchyPanel() :
    Panel() {}

sptr<Panel> SceneHierarchyPanel::make() {

    auto panel { _STD shared_ptr<SceneHierarchyPanel>(new SceneHierarchyPanel()) };
    return panel;

}
