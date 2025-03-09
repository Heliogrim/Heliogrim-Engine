#pragma once

#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>

namespace hg::editor::ui {
	class EditorSceneViewportPanel :
		public engine::reflow::uikit::VerticalLayout {
	protected:
		EditorSceneViewportPanel();

	public:
		static sptr<EditorSceneViewportPanel> make();
	};
}
