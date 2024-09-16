#include "PreviewSceneViewportPanel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

PreviewSceneViewportPanel::PreviewSceneViewportPanel() :
	VerticalPanel() {}

sptr<PreviewSceneViewportPanel> PreviewSceneViewportPanel::make() {
	auto panel { std::shared_ptr<PreviewSceneViewportPanel>(new PreviewSceneViewportPanel()) };
	return panel;
}
