#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::editor {
	class EditorUI;
}

namespace hg::editor::ui {
	void initEditor(_In_ ref<EditorUI> editorUi_);
}
