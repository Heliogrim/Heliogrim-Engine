#include "EditorTheme.hpp"

#include <Editor.UI.Main/EditorUI.hpp>
#include <Editor.UI.Main/Module/EditorUI.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow/Reflow.hpp>
#include <Engine.Reflow/Module/Reflow.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

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

/**/

/**
 * Section : Basis
 */

#pragma region Base

static Theme generate_base_theme() {

	auto theme = theming::Theme {};
	const auto fontGuid = static_cast<ptr<editor::EditorUI>>(
		engine::Engine::getEngine()->getModules().getSubModule(editor::EditorUIDepKey).get()
	)->getDefaultFontGuid();

	// TODO: Cleanup!?
	auto fontTypedGuid = TypedAssetGuid<engine::assets::Font> {};
	*static_cast<AssetGuid*>(static_cast<void*>(&fontTypedGuid)) = fontGuid;

	/**/

	theme.addImageStyle("[Image]"sv, ImageStyleAttributes { styles.base.normal.image_alpha, styles.base.normal.image_tint });

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.base.normal.color_background, 8.F });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.base.normal.color_background, 8.F });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.base.normal.color_background, 0.F });

	theme.addTextStyle(
		"[Text]"sv,
		TextStyleAttributes {
			fontTypedGuid,
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

static theming::Theme generate_lvl1_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 8.F });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl1.normal.color_background, 8.F });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	/**/

	return theme;
}

static Theme generate_lvl1_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl1.hover.color_background, 8.F });

	/**/

	return theme;
}

#pragma endregion Lvl1

/**
 * Section : Elevation II
 */

#pragma region Lvl2

static Theme generate_lvl2_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 8.F });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 8.F });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	/**/

	return theme;
}

static Theme generate_lvl2_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl2.hover.color_background, 8.F });

	/**/

	return theme;
}

#pragma endregion

/**
 * Section : Elevation III
 */

#pragma region Lvl3

static Theme generate_lvl3_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl3.normal.color_background, 8.F });
	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl3.normal.color_background, 8.F });

	/**/

	return theme;
}

static Theme generate_lvl3_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[Button] > background"sv, PaintStyleAttributes { styles.lvl3.hover.color_background, 8.F });

	/**/

	return theme;
}

#pragma endregion Lvl3

/**/

void editor::ui::setupEditorTheme(ref<Theming> theming_) {

	theming_.storeBaseTheme(generate_base_theme());

	/**/

	theming_.storeStatedTheme(1u, {}, generate_lvl1_theme());
	theming_.storeStatedTheme(1u, { .hover = true }, generate_lvl1_hover_theme());

	/**/

	theming_.storeStatedTheme(2u, {}, generate_lvl2_theme());
	theming_.storeStatedTheme(2u, { .hover = true }, generate_lvl2_hover_theme());

	/**/

	theming_.storeStatedTheme(3u, {}, generate_lvl3_theme());
	theming_.storeStatedTheme(3u, { .hover = true }, generate_lvl3_hover_theme());

}

ref<const Theming> editor::ui::getEditorTheming() {
	const auto reflowModule = engine::Engine::getEngine()->getModules().getSubModule(engine::ReflowDepKey);
	::hg::assertrt(reflowModule != nullptr);
	return static_cast<ref<const engine::Reflow>>(*reflowModule).getTheming();
}
