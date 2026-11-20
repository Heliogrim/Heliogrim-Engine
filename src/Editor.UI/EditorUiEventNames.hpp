#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::ui {
	struct EditorUiEventNames {
		constexpr static auto ContextMenuEvent = "ContextMenuEvent"sv;
		constexpr static auto InputChangeEvent = "InputChangeEvent"sv;
		constexpr static auto SelectMenuEvent = "SelectMenuEvent"sv;
	};
}
