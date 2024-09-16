#pragma once
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
	class GameplaySceneViewportPanel :
		public engine::reflow::VerticalPanel {
	protected:
		GameplaySceneViewportPanel();

	public:
		static sptr<GameplaySceneViewportPanel> make();
	};
}
