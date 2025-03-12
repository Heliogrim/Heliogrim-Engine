#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow.Theming/Theming.hpp>

namespace hg::editor::ui {
	extern void setupEditorTheme(_Inout_ ref<engine::reflow::theming::Theming> theming_);

	extern ref<const engine::reflow::theming::Theming> getEditorTheming();
}
