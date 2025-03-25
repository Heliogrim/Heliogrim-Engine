#pragma once

#include <Engine.Common/String.hpp>

namespace hg::editor::ui {
	struct EditorUiEventNames {
		constexpr static auto ContextMenuEvent = "ContextMenuEvent"sv;
		constexpr static auto InputChangeEvent = "InputChangeEvent"sv;
	};
}
