#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
#endif

namespace hg::editor::ui {
	extern void setupEditorTheme(_Inout_ ref<engine::reflow::theming::Theming> theming_);

	extern ref<const engine::reflow::theming::Theming> getEditorTheming();
}
