#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow.Theming/Theme.hpp>

namespace hg::engine::reflow::uikit {

	extern theming::Theme generate_base_theme();

	extern theming::Theme generate_lvl1_theme();
	extern theming::Theme generate_lvl1_hover_theme();

	extern theming::Theme generate_lvl2_theme();
	extern theming::Theme generate_lvl2_hover_theme();

	extern theming::Theme generate_lvl3_theme();
	extern theming::Theme generate_lvl3_hover_theme();
}
