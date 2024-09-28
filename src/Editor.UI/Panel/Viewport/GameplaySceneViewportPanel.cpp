#include "GameplaySceneViewportPanel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

GameplaySceneViewportPanel::GameplaySceneViewportPanel() :
	VerticalPanel() {}

sptr<GameplaySceneViewportPanel> GameplaySceneViewportPanel::make() {
	auto panel { std::shared_ptr<GameplaySceneViewportPanel>(new GameplaySceneViewportPanel()) };
	return panel;
}
