#include "SceneHierarchyPanel.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

SceneHierarchyPanel::SceneHierarchyPanel() :
    Panel(nullptr) {}

sptr<Panel> SceneHierarchyPanel::make() {

    auto panel { _STD shared_ptr<SceneHierarchyPanel>(new SceneHierarchyPanel()) };
    return panel;

}
