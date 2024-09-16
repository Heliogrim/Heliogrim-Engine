#pragma once
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
	class PreviewSceneViewportPanel :
		public engine::reflow::VerticalPanel {
	protected:
		PreviewSceneViewportPanel();

	public:
		static sptr<PreviewSceneViewportPanel> make();
	};
}
