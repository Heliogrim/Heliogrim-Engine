#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
#endif

namespace hg::editor::ui {
	class SceneEditorController;
}

namespace hg::editor::ui {
	struct SceneEditorView {
		SharedPtr<engine::reflow::Widget> main;
	};

	[[nodiscard]] extern UniquePtr<SceneEditorView> makeSceneEditorView(
		ref<const SceneEditorController> controller_
	);
}
