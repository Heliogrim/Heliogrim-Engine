#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor {
	class EditorUI;
}

namespace hg::editor::ui {
	void initEditor(_In_ ref<EditorUI> editorUi_);
}
