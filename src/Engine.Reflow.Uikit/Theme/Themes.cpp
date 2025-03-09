#include "Themes.hpp"

#include "Editor.UI.Main/EditorUI.hpp"
#include "Editor.UI.Main/Module/EditorUI.hpp"
#include "Engine.Core/Engine.hpp"
#include "Engine.Core/Module/Modules.hpp"

using namespace hg::engine::reflow::uikit;
using namespace hg::engine::reflow;
using namespace hg;

static const auto stable_ref_theme = theming::Theme {};

/**/

constexpr static struct StyleSystem {
	struct BaseStyles {
		struct Normal {
			engine::color color_background { 33.F, 37.F, 43.F, 255.F };
			engine::color color_font { 255.F, 255.F, 255.F, 255.F };

			Padding box_padding = Padding { 8.F };
			ReflowPlacement box_placement = ReflowPlacement::eMiddleCenter;

			ReflowAlignment flex_align = ReflowAlignment::eCenter;
			ReflowSpacing flex_spacing = ReflowSpacing::eStart;
			Padding flex_padding = Padding { 0.F };

			f32 font_size = 12.F;
			f32 text_line_height = 1.F;
			TextAlign text_align = TextAlign::eTopLeft;
			ReflowWrap text_wrap = ReflowWrap::eNone;

			ReflowAlphaMode image_alpha = ReflowAlphaMode::eNone;
			engine::color image_tint = engine::color { 255.F, 255.F, 255.F, 255.F };

			Padding input_padding = Padding { 4.F };
			TextAlign input_text_align = TextAlign::eMiddleLeft;
		} normal;

		struct Hover {} hover;
	} base;

	struct Lvl1Styles {
		struct Normal {
			engine::color color_background { 57.F, 67.F, 81.F, 255.F };
		} normal;

		struct Hover {
			engine::color color_background { 42.F, 49.F, 59.F, 255.F };
		} hover;
	} lvl1;

	struct Lvl2Styles {
		struct Normal {
			engine::color color_background { 46.F, 52.F, 61.F, 255.F };
		} normal;

		struct Hover {
			engine::color color_background { 61.F, 69.F, 80.F, 255.F };
		} hover;
	} lvl2;

	struct Lvl3Styles {
		struct Normal {
			engine::color color_background { 25.F, 27.F, 31.F, 255.F };
		} normal;

		struct Hover {
			engine::color color_background { 33.F, 36.F, 41.F, 255.F };
		} hover;
	} lvl3;
} styles;

/**
 * Section : Basis
 */

#pragma region Base

theming::Theme uikit::generate_base_theme() {

	auto theme = theming::Theme {};
	auto font = static_cast<ptr<editor::EditorUI>>(
		Engine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	)->getDefaultFont();

	/**/

	theme.addImageStyle("[Image]"sv, ImageStyleAttributes { styles.base.normal.image_alpha, styles.base.normal.image_tint });

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.base.normal.color_background });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.base.normal.color_background });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.base.normal.color_background });

	theme.addTextStyle(
		"[Text]"sv,
		TextStyleAttributes {
			*font,
			styles.base.normal.font_size,
			styles.base.normal.text_line_height,
			styles.base.normal.text_align,
			0uL,
			styles.base.normal.text_wrap,
			styles.base.normal.color_font
		}
	);

	/**/

	return theme;
}

#pragma endregion Base

/**
 * Section : Elevation I
 */

#pragma region Lvl1

theming::Theme uikit::generate_lvl1_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl1.normal.color_background });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background });

	/**/

	return theme;
}

theming::Theme uikit::generate_lvl1_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl1.hover.color_background });

	/**/

	return theme;
}

#pragma endregion Lvl1

/**
 * Section : Elevation II
 */

#pragma region Lvl2

theming::Theme uikit::generate_lvl2_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background });

	/**/

	return theme;
}

theming::Theme uikit::generate_lvl2_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.hover.color_background });

	/**/

	return theme;
}

#pragma endregion

/**
 * Section : Elevation III
 */

#pragma region Lvl3

theming::Theme uikit::generate_lvl3_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl3.normal.color_background });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl3.normal.color_background });

	/**/

	return theme;
}

theming::Theme uikit::generate_lvl3_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl3.hover.color_background });

	/**/

	return theme;
}

#pragma endregion Lvl3
