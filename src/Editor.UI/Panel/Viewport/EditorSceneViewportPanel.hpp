#pragma once
#include <Engine.Reflow/Widget/VerticalPanel.hpp>

namespace hg::editor::ui {
	class EditorSceneViewportPanel :
		public engine::reflow::VerticalPanel {
	protected:
		EditorSceneViewportPanel();

	public:
		static sptr<EditorSceneViewportPanel> make();
	};
}
