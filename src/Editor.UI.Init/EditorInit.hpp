#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor {
	class EditorUI;
}

namespace hg::editor::ui {
	void initEditor(_In_ ref<EditorUI> editorUi_);
}

namespace hg {
	class Actor;
	class HierarchyComponent;
}

namespace hg::editor::ui {
	/* Attention: Temporary */
	void storeEditorSelectedTarget(ref<EditorUI> editorUI_, const hg::ptr<hg::Actor> target_);

	void storeEditorSelectedTarget(ref<EditorUI> editorUI_, const hg::ptr<hg::HierarchyComponent> target_);
}
