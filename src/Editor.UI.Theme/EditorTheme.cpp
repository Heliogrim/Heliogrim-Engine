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

constexpr static struct Colors {
	engine::color night { 17.F, 21.F, 23.F, 255.F };
	engine::color stygian { 26.F, 32.F, 35.F, 255.F };
	engine::color gunmetal { 35.F, 43.F, 47.F, 255.F };
	engine::color metallic_charcoal { 50.F, 62.F, 67.F, 255.F };
	engine::color caribbean { 22.F, 99.F, 116.F, 255.F };
	engine::color french_grey { 195.F, 195.F, 201.F, 255.F };
	engine::color ghost_white { 249.F, 249.F, 255.F, 255.F };
} colors;

constexpr static struct StyleSystem {
	struct RootStyles {
		engine::color window_background = colors.gunmetal;
		engine::color card_background = colors.stygian;

		engine::color input_background = colors.gunmetal;
		engine::color input_active_background = colors.night;
		engine::color input_foreground = colors.french_grey;
		engine::color input_active_foreground = colors.ghost_white;

		engine::color button_background = colors.gunmetal;
		engine::color button_active_background = colors.night;
		engine::color button_foreground = colors.french_grey;
		engine::color button_active_foreground = colors.ghost_white;

		engine::color menu_background = button_background;

		engine::color browser_background = colors.gunmetal;
		engine::color browser_item_background = colors.night;
		engine::color browser_active_item_background = colors.caribbean;
		engine::color browser_item_foreground = colors.french_grey;
		engine::color browser_active_item_foreground = colors.ghost_white;

		engine::color hierarchy_background = colors.stygian;
		engine::color hierarchy_item_background = colors.stygian;
		engine::color hierarchy_active_item_background = colors.caribbean;
		engine::color hierarchy_selected_item_background = colors.metallic_charcoal;

		engine::color text_foreground = colors.ghost_white;

		/**/

		f32 structure_borderRadius = 8.F;
		f32 interactive_borderRadius = 4.F;
	} root;

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
			engine::color color_background = colors.stygian;
		} normal;

		struct Hover {
			engine::color color_background { 61.F, 69.F, 80.F, 255.F };
		} hover;

		struct Select {
			engine::color color_background = colors.metallic_charcoal;
		} select;
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
	auto fontTypedGuid = TypedAssetGuid<engine::assets::FontAsset> {};
	*static_cast<AssetGuid*>(static_cast<void*>(&fontTypedGuid)) = fontGuid;

	/**/

	theme.addImageStyle("[Image]"sv, ImageStyleAttributes { styles.base.normal.image_alpha, styles.base.normal.image_tint });

	theme.addPaintStyle(
		"[Button] > background"sv,
		PaintStyleAttributes { styles.root.button_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[Card] > background"sv,
		PaintStyleAttributes { styles.root.card_background, styles.root.structure_borderRadius }
	);
	theme.addPaintStyle(
		"[Input] > background"sv,
		PaintStyleAttributes { styles.root.input_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[PropertySectionHeader]"sv,
		PaintStyleAttributes { styles.root.button_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[Menu] > background"sv,
		PaintStyleAttributes { styles.root.menu_background, styles.root.structure_borderRadius }
	);
	theme.addPaintStyle(
		"[MenuSpacer] > line"sv,
		PaintStyleAttributes { styles.root.button_foreground, 0.F }
	);
	theme.addPaintStyle(
		"[AssetBrowser]"sv,
		PaintStyleAttributes { styles.root.browser_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[AssetBrowserItem]"sv,
		PaintStyleAttributes { styles.root.browser_item_background, styles.root.interactive_borderRadius }
	);

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.base.normal.color_background, 0.F });
	theme.addPaintStyle("[Test]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	theme.addPaintStyle("[SceneHierarchy]"sv, PaintStyleAttributes { styles.root.hierarchy_background, 0.F });
	theme.addPaintStyle("[SceneHierarchyItem]"sv, PaintStyleAttributes { styles.root.hierarchy_item_background, 0.F });

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

	theme.addTextStyle(
		"[MenuItem] > title"sv,
		TextStyleAttributes {
			fontTypedGuid,
			styles.base.normal.font_size,
			styles.base.normal.text_line_height,
			TextAlign::eMiddleLeft,
			0uL,
			ReflowWrap::eNone,
			styles.base.normal.color_font
		}
	);

	theme.addTextStyle(
		"[AssetBrowserItem] > title"sv,
		TextStyleAttributes {
			fontTypedGuid,
			styles.base.normal.font_size,
			styles.base.normal.text_line_height,
			styles.base.normal.text_align,
			1uL,
			ReflowWrap::eNoWrap,
			styles.root.browser_active_item_foreground
		}
	);

	theme.addTextStyle(
		"[AssetBrowserItem] > label"sv,
		TextStyleAttributes {
			fontTypedGuid,
			styles.base.normal.font_size,
			styles.base.normal.text_line_height,
			styles.base.normal.text_align,
			1uL,
			ReflowWrap::eNoWrap,
			styles.root.browser_item_foreground
		}
	);

	/**/

	return theme;
}

static theming::Theme generate_base_hover_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle(
		"[Button] > background"sv,
		PaintStyleAttributes { styles.root.button_active_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[Input] > background"sv,
		PaintStyleAttributes { styles.root.input_active_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[PropertySectionHeader]"sv,
		PaintStyleAttributes { styles.root.button_active_background, styles.root.interactive_borderRadius }
	);
	theme.addPaintStyle(
		"[AssetBrowserItem]"sv,
		PaintStyleAttributes { styles.root.browser_active_item_background, styles.root.interactive_borderRadius }
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

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	/**/

	return theme;
}

static Theme generate_lvl1_hover_theme() {
	return theming::Theme {};
}

#pragma endregion Lvl1

/**
 * Section : Elevation II
 */

#pragma region Lvl2

static Theme generate_lvl2_theme() {

	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[TreeViewItem] > background"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { styles.lvl2.normal.color_background, 0.F });

	/**/

	return theme;
}

static Theme generate_lvl2_hover_theme() {
	auto theme = theming::Theme {};

	/**/

	/**/

	return theme;
}

static Theme generate_lvl2_selected_theme() {
	auto theme = theming::Theme {};

	/**/

	theme.addPaintStyle("[TreeViewItem] > background"sv, PaintStyleAttributes { styles.lvl2.select.color_background, 0.F });

	/**/

	return theme;
}

static Theme generate_lvl2_focus_theme() {
	auto theme = theming::Theme();
	/**/

	theme.addPaintStyle("[TreeViewItem] > background"sv, PaintStyleAttributes { styles.root.hierarchy_active_item_background, 0.F });

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

	theme.addPaintStyle("[Card] > background"sv, PaintStyleAttributes { styles.lvl3.normal.color_background, 8.F });

	return theme;
}

static Theme generate_lvl3_hover_theme() {
	auto theme = theming::Theme {};
	return theme;
}

#pragma endregion Lvl3

/**/

void editor::ui::setupEditorTheme(ref<Theming> theming_) {

	theming_.storeBaseTheme(generate_base_theme());

	theming_.storeStatedTheme(0u, { .hover = true }, generate_base_hover_theme());

	/**/

	theming_.storeStatedTheme(1u, {}, generate_lvl1_theme());
	theming_.storeStatedTheme(1u, { .hover = true }, generate_lvl1_hover_theme());

	/**/

	theming_.storeStatedTheme(2u, {}, generate_lvl2_theme());
	theming_.storeStatedTheme(2u, { .focus = true }, generate_lvl2_focus_theme());
	theming_.storeStatedTheme(2u, { .hover = true }, generate_lvl2_hover_theme());
	theming_.storeStatedTheme(2u, { .selected = true }, generate_lvl2_selected_theme());

	/**/

	theming_.storeStatedTheme(3u, {}, generate_lvl3_theme());
	theming_.storeStatedTheme(3u, { .hover = true }, generate_lvl3_hover_theme());

}

ref<const Theming> editor::ui::getEditorTheming() {
	const auto reflowModule = engine::Engine::getEngine()->getModules().getSubModule(engine::ReflowDepKey);
	::hg::assertrt(reflowModule != nullptr);
	return static_cast<ref<const engine::Reflow>>(*reflowModule).getTheming();
}
