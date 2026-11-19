#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Reflow.Theming/Theming.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui {
	extern void setupEditorTheme(_Inout_ ref<engine::reflow::theming::Theming> theming_);

	extern ref<const engine::reflow::theming::Theming> getEditorTheming();
}
