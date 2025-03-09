#pragma once

#include <Engine.Common/Memory/SharedPointer.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

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
