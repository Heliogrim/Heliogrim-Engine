#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow.Uikit;
#endif

namespace hg::editor::ui {
	class EditorSceneViewportPanel :
		public engine::reflow::uikit::VerticalLayout {
	protected:
		EditorSceneViewportPanel();

	public:
		static sptr<EditorSceneViewportPanel> make();
	};
}
