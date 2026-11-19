#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
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
