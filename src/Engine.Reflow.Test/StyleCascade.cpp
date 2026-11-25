#include "pch.h"

/**/

#include "TreeFixture.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	namespace {
		[[nodiscard]] BoxLayoutAttributes legacyBox(const float base_);

		[[nodiscard]] TypedAssetGuid<engine::assets::FontAsset> themedFont();

		void expectBox(ref<const BoxLayoutAttributes> box_, const float base_);

		[[nodiscard]] Vector<theming::Theme> singleTheme(mref<theming::Theme> theme_);

		[[nodiscard]] sptr<uikit::ThemeProvider> mountThemed(ref<const sptr<Widget>> child_, mref<theming::Theme> theme_);

		[[nodiscard]] sptr<uikit::ThemeProvider> mountNested(
			ref<const sptr<Widget>> child_,
			mref<theming::Theme> outer_,
			mref<theming::Theme> inner_
		);

		[[nodiscard]] engine::color red();

		[[nodiscard]] engine::color green();

		[[nodiscard]] engine::color blue();

		[[nodiscard]] engine::color gray();

		[[nodiscard]] PaintStyleAttributes storedPaint();

		[[nodiscard]] theming::StateLayer hoverTint(const InteractiveStateFlagBits bit_, ref<const engine::color> tint_);

		[[nodiscard]] theming::ComponentStyle withLayers(mref<Vector<theming::StateLayer>> layers_);

		void resolvePaint(
			ref<const theming::Theme> theme_,
			const StyleStates states_,
			ref<const theming::StyleRule<PaintStyleAttributes>> instance_,
			ref<PaintStyleAttributes> inOut_
		);
	}

	/**/

	TEST(StyleCascade, LegacyPaintSlotsUnchanged) {
		for (const auto matching : { true, false }) {
			auto paint = make_sptr<uikit::Paint>();
			paint->getStyleAttributes().update<attr::PaintStyle::tint>(engine::color { 1.F, 0.F, 0.F, 1.F });
			paint->getStyleAttributes().update<attr::PaintStyle::borderRadius>(math::fvec4 { 1.F, 1.F, 1.F, 1.F });
			paint->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 3.F });

			theming::Theme theme {};
			const auto name = matching ? "[Paint]"sv : "other"sv;
			theme.addPaintStyle(name, PaintStyleAttributes { engine::color { 0.F, 1.F, 0.F, 1.F }, math::fvec4 { 2.F, 3.F, 4.F, 5.F } });
			theme.addBoxLayout(name, legacyBox(10.F));
			const auto provider = mountThemed(paint, ::hg::move(theme));

			const auto& style = paint->getStyleAttributes();
			if (matching) {
				EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), (engine::color { 0.F, 1.F, 0.F, 1.F }));
				EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 2.F, 3.F, 4.F, 5.F }));
			} else {
				EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), (engine::color { 1.F, 0.F, 0.F, 1.F }));
				EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 1.F, 1.F, 1.F, 1.F }));
			}
			// Paint never copied box layout from a theme.
			EXPECT_EQ(paint->getLayoutAttributes().valueOf<attr::BoxLayout::padding>(), Padding { 3.F });
		}
	}

	TEST(StyleCascade, LegacyTextSlotsUnchanged) {
		for (const auto matching : { true, false }) {
			auto text = make_sptr<uikit::Text>();
			// Style and box match through different class names of the same atom.
			text->getStaticClassList().add("[Label]"sv);
			auto& stored = text->getStyleAttributes();
			stored.update<attr::TextStyle::fontSize>(12.F);
			stored.update<attr::TextStyle::lineHeight>(1.F);
			stored.update<attr::TextStyle::textAlign>(TextAlign::eTopLeft);
			stored.update<attr::TextStyle::textEllipse>(0u);
			stored.update<attr::TextStyle::textWrap>(ReflowWrap::eNone);
			stored.update<attr::TextStyle::color>(engine::color { 1.F, 1.F, 1.F, 1.F });
			text->getLayoutAttributes() = legacyBox(1.F);

			theming::Theme theme {};
			const auto styleName = matching ? "[Text]"sv : "other"sv;
			const auto boxName = matching ? "[Label]"sv : "other"sv;
			theme.addTextStyle(
				styleName,
				TextStyleAttributes {
					themedFont(), 20.F, 2.F, TextAlign::eMiddleRight, 3u, ReflowWrap::eWrap,
					engine::color { 0.F, 0.F, 1.F, 1.F }
				}
			);
			theme.addBoxLayout(boxName, legacyBox(10.F));
			const auto provider = mountThemed(text, ::hg::move(theme));

			const auto& style = text->getStyleAttributes();
			if (matching) {
				EXPECT_EQ(static_cast<AssetGuid>(style.valueOf<attr::TextStyle::font>()), static_cast<AssetGuid>(themedFont()));
				EXPECT_EQ(style.valueOf<attr::TextStyle::fontSize>(), 20.F);
				EXPECT_EQ(style.valueOf<attr::TextStyle::lineHeight>(), 2.F);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textAlign>(), TextAlign::eMiddleRight);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textEllipse>(), 3u);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textWrap>(), ReflowWrap::eWrap);
				EXPECT_EQ(style.valueOf<attr::TextStyle::color>(), (engine::color { 0.F, 0.F, 1.F, 1.F }));
				expectBox(text->getLayoutAttributes(), 10.F);
			} else {
				EXPECT_EQ(static_cast<AssetGuid>(style.valueOf<attr::TextStyle::font>()), invalid_asset_guid);
				EXPECT_EQ(style.valueOf<attr::TextStyle::fontSize>(), 12.F);
				EXPECT_EQ(style.valueOf<attr::TextStyle::lineHeight>(), 1.F);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textAlign>(), TextAlign::eTopLeft);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textEllipse>(), 0u);
				EXPECT_EQ(style.valueOf<attr::TextStyle::textWrap>(), ReflowWrap::eNone);
				EXPECT_EQ(style.valueOf<attr::TextStyle::color>(), (engine::color { 1.F, 1.F, 1.F, 1.F }));
				expectBox(text->getLayoutAttributes(), 1.F);
			}
		}
	}

	TEST(StyleCascade, LegacyImageSlotsUnchanged) {
		for (const auto matching : { true, false }) {
			auto image = make_sptr<uikit::Image>();
			image->getStyleAttributes().update<attr::ImageStyle::alpha>(ReflowAlphaMode::eNone);
			image->getStyleAttributes().update<attr::ImageStyle::tint>(engine::color { 1.F, 1.F, 1.F, 1.F });
			image->getLayoutAttributes() = legacyBox(1.F);

			theming::Theme theme {};
			const auto name = matching ? "[Image]"sv : "other"sv;
			theme.addImageStyle(name, ImageStyleAttributes { ReflowAlphaMode::eEnable, engine::color { 0.F, 1.F, 1.F, 1.F } });
			theme.addBoxLayout(name, legacyBox(10.F));
			const auto provider = mountThemed(image, ::hg::move(theme));

			const auto& style = image->getStyleAttributes();
			if (matching) {
				EXPECT_EQ(style.valueOf<attr::ImageStyle::alpha>(), ReflowAlphaMode::eEnable);
				EXPECT_EQ(style.valueOf<attr::ImageStyle::tint>(), (engine::color { 0.F, 1.F, 1.F, 1.F }));
			} else {
				EXPECT_EQ(style.valueOf<attr::ImageStyle::alpha>(), ReflowAlphaMode::eNone);
				EXPECT_EQ(style.valueOf<attr::ImageStyle::tint>(), (engine::color { 1.F, 1.F, 1.F, 1.F }));
			}
			// The image layout copy is still a TODO, so the stored layout survives either theme.
			expectBox(image->getLayoutAttributes(), 1.F);
		}
	}

	TEST(StyleCascade, LegacyThemeStillOverridesStoredValue) {
		auto paint = make_sptr<uikit::Paint>();
		paint->getStyleAttributes() = storedPaint();

		theming::Theme theme {};
		theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { green(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		const auto provider = mountThemed(paint, ::hg::move(theme));

		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::tint>(), green());
		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 2.F, 2.F, 2.F, 2.F }));
	}

	TEST(StyleCascade, ParentContextOnlyMatchApplies) {
		auto paint = make_sptr<uikit::Paint>();
		paint->getStyleAttributes() = storedPaint();

		theming::Theme outer {};
		outer.addPaintStyle("[Paint]"sv, PaintStyleAttributes { green(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		theming::Theme inner {};
		inner.addPaintStyle("other"sv, PaintStyleAttributes { blue(), math::fvec4 { 3.F, 3.F, 3.F, 3.F } });
		const auto provider = mountNested(paint, ::hg::move(outer), ::hg::move(inner));

		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::tint>(), green());
		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 2.F, 2.F, 2.F, 2.F }));
	}

	TEST(StyleCascade, LocalThemeOverridesParent) {
		auto paint = make_sptr<uikit::Paint>();
		paint->getStyleAttributes() = storedPaint();

		theming::Theme outer {};
		outer.addPaintStyle("[Paint]"sv, PaintStyleAttributes { green(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		theming::Theme inner {};
		inner.addPaintStyle("[Paint]"sv, PaintStyleAttributes { blue(), math::fvec4 { 3.F, 3.F, 3.F, 3.F } });
		const auto provider = mountNested(paint, ::hg::move(outer), ::hg::move(inner));

		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::tint>(), blue());
		EXPECT_EQ(paint->getStyleAttributes().valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 3.F, 3.F, 3.F, 3.F }));
	}

	TEST(StyleCascade, NoMatchKeepsStoredValue) {
		theming::Theme theme {};
		theme.addPaintStyle("other"sv, PaintStyleAttributes { green(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		theme.addComponentStyle("other"sv, withLayers({ hoverTint(InteractiveStateFlagBits::eHover, green()) }));

		auto style = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover }, {}, style);

		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), red());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 1.F, 1.F, 1.F, 1.F }));
	}

	TEST(StyleCascade, HoverAndSelectedBothApply) {
		auto component = withLayers({ hoverTint(InteractiveStateFlagBits::eHover, green()) });
		component.paint.with<attr::PaintStyle::tint>(blue());
		theming::StateLayer selected { .mask = StyleStates { InteractiveStateFlagBits::eSelected } };
		selected.paint.with<attr::PaintStyle::borderRadius>(math::fvec4 { 9.F, 9.F, 9.F, 9.F });
		component.states.emplace_back(::hg::move(selected));

		theming::Theme theme {};
		theme.addComponentStyle("[Btn]"sv, ::hg::move(component));

		auto style = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover } | InteractiveStateFlagBits::eSelected, {}, style);

		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), green());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 9.F, 9.F, 9.F, 9.F }));

		/**/

		auto idle = storedPaint();
		resolvePaint(theme, {}, {}, idle);
		EXPECT_EQ(idle.valueOf<attr::PaintStyle::tint>(), blue());
		EXPECT_EQ(idle.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 1.F, 1.F, 1.F, 1.F }));
	}

	TEST(StyleCascade, DisabledOverridesHoverOnSharedSlot) {
		// Declared disable-first, so the result depends on the precedence sort, not on insertion order.
		theming::Theme theme {};
		theme.addComponentStyle(
			"[Btn]"sv,
			withLayers({ hoverTint(InteractiveStateFlagBits::eDisable, gray()), hoverTint(InteractiveStateFlagBits::eHover, green()) })
		);

		auto style = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover } | InteractiveStateFlagBits::eDisable, {}, style);
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), gray());

		auto hovered = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover }, {}, hovered);
		EXPECT_EQ(hovered.valueOf<attr::PaintStyle::tint>(), green());
	}

	TEST(StyleCascade, FocusedAndInvalidCombine) {
		auto component = withLayers({ hoverTint(InteractiveStateFlagBits::eInvalid, red()) });
		theming::StateLayer focus { .mask = StyleStates { InteractiveStateFlagBits::eFocus } };
		focus.paint.with<attr::PaintStyle::borderRadius>(math::fvec4 { 3.F, 3.F, 3.F, 3.F });
		focus.paint.with<attr::PaintStyle::tint>(blue());
		component.states.emplace_back(::hg::move(focus));

		theming::Theme theme {};
		theme.addComponentStyle("[Btn]"sv, ::hg::move(component));

		auto style = storedPaint();
		style.update<attr::PaintStyle::tint>(gray());
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eFocus } | InteractiveStateFlagBits::eInvalid, {}, style);

		// Invalid outranks focus on the shared tint; the focus-only radius still applies.
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), red());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 3.F, 3.F, 3.F, 3.F }));
	}

	TEST(StyleCascade, InstanceOverrideWins) {
		theming::Theme theme {};
		theme.addPaintStyle("[Btn]"sv, PaintStyleAttributes { gray(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		theme.addComponentStyle("[Btn]"sv, withLayers({ hoverTint(InteractiveStateFlagBits::eDisable, green()) }));

		theming::StyleRule<PaintStyleAttributes> instance {};
		instance.with<attr::PaintStyle::tint>(blue());

		auto style = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eDisable }, instance, style);

		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), blue());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), (math::fvec4 { 2.F, 2.F, 2.F, 2.F }));
	}

	TEST(StyleCascade, SelectedHoverMaskLayerWins) {
		auto both = hoverTint(InteractiveStateFlagBits::eSelected, gray());
		both.mask = StyleStates { InteractiveStateFlagBits::eSelected } | InteractiveStateFlagBits::eHover;

		theming::Theme theme {};
		theme.addComponentStyle(
			"[Btn]"sv,
			withLayers(
				{ ::hg::move(both), hoverTint(InteractiveStateFlagBits::eHover, green()), hoverTint(InteractiveStateFlagBits::eSelected, blue()) }
			)
		);

		auto style = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover } | InteractiveStateFlagBits::eSelected, {}, style);
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), gray());

		auto hovered = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eHover }, {}, hovered);
		EXPECT_EQ(hovered.valueOf<attr::PaintStyle::tint>(), green());

		auto selected = storedPaint();
		resolvePaint(theme, StyleStates { InteractiveStateFlagBits::eSelected }, {}, selected);
		EXPECT_EQ(selected.valueOf<attr::PaintStyle::tint>(), blue());
	}

	class StyleCascadeTree :
		public TreeFixture {};

	TEST_F(StyleCascadeTree, UnchangedResolveMarksNothing) {
		theming::Theme theme {};
		theme.addPaintStyle("[Btn]"sv, PaintStyleAttributes { green(), math::fvec4 { 2.F, 2.F, 2.F, 2.F } });
		theme.addComponentStyle("[Btn]"sv, withLayers({ hoverTint(InteractiveStateFlagBits::eHover, blue()) }));

		const auto probe = make_sptr<ProbePaint>();
		const auto states = StyleStates { InteractiveStateFlagBits::eHover };

		resolvePaint(theme, states, {}, probe->getStyleAttributes());
		EXPECT_EQ(probe->getStyleAttributes().valueOf<attr::PaintStyle::tint>(), blue());
		tick(probe);
		ASSERT_FALSE(probe->state().isRenderPending());

		resolvePaint(theme, states, {}, probe->getStyleAttributes());
		EXPECT_FALSE(probe->state().isRenderPending());
		EXPECT_FALSE(probe->state().isLayoutPending());
	}

	/**/

	namespace {
		BoxLayoutAttributes legacyBox(const float base_) {
			return BoxLayoutAttributes {
				ReflowUnit { ReflowUnitType::eAbsolute, base_ }, ReflowUnit { ReflowUnitType::eAbsolute, base_ + 1.F }, base_ + 2.F, base_ + 3.F,
				ReflowUnit { ReflowUnitType::eRelative, base_ + 4.F }, ReflowUnit { ReflowUnitType::eRelative, base_ + 5.F }, base_ + 6.F, base_ + 7.F,
				Padding { base_ + 8.F }, base_ > 5.F ? ReflowPlacement::eBottomRight : ReflowPlacement::eTopLeft
			};
		}

		TypedAssetGuid<engine::assets::FontAsset> themedFont() {
			// Strict TypedAssetGuid has no guid constructor; same write as EditorTheme.cpp.
			auto font = TypedAssetGuid<engine::assets::FontAsset> {};
			*static_cast<AssetGuid*>(static_cast<void*>(&font)) = AssetGuid { Guid { 1u, 2u, 3u, 4uLL } };
			return font;
		}

		void expectBox(ref<const BoxLayoutAttributes> box_, const float base_) {
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::minWidth>(), (ReflowUnit { ReflowUnitType::eAbsolute, base_ }));
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::maxWidth>(), (ReflowUnit { ReflowUnitType::eAbsolute, base_ + 1.F }));
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::widthGrow>(), base_ + 2.F);
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::widthShrink>(), base_ + 3.F);
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::minHeight>(), (ReflowUnit { ReflowUnitType::eRelative, base_ + 4.F }));
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::maxHeight>(), (ReflowUnit { ReflowUnitType::eRelative, base_ + 5.F }));
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::heightGrow>(), base_ + 6.F);
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::heightShrink>(), base_ + 7.F);
			EXPECT_EQ(box_.valueOf<attr::BoxLayout::padding>(), Padding { base_ + 8.F });
			EXPECT_EQ(
				box_.valueOf<attr::BoxLayout::placement>(),
				base_ > 5.F ? ReflowPlacement::eBottomRight : ReflowPlacement::eTopLeft
			);
		}

		Vector<theming::Theme> singleTheme(mref<theming::Theme> theme_) {
			Vector<theming::Theme> themes {};
			themes.emplace_back(::hg::move(theme_));
			return themes;
		}

		sptr<uikit::ThemeProvider> mountThemed(ref<const sptr<Widget>> child_, mref<theming::Theme> theme_) {
			auto provider = make_sptr<uikit::ThemeProvider>();
			provider->setChild(child_);
			provider->setProviderThemes(singleTheme(::hg::move(theme_)));
			// A second pass, so the child reads its effective provisioner even if the first pass refreshed it late.
			provider->cascadeContextChange(false);
			return provider;
		}

		sptr<uikit::ThemeProvider> mountNested(
			ref<const sptr<Widget>> child_,
			mref<theming::Theme> outer_,
			mref<theming::Theme> inner_
		) {
			auto inner = make_sptr<uikit::ThemeProvider>();
			inner->setChild(child_);
			inner->setProviderThemes(singleTheme(::hg::move(inner_)));
			auto outer = mountThemed(inner, ::hg::move(outer_));
			return outer;
		}

		engine::color red() {
			return engine::color { 1.F, 0.F, 0.F, 1.F };
		}

		engine::color green() {
			return engine::color { 0.F, 1.F, 0.F, 1.F };
		}

		engine::color blue() {
			return engine::color { 0.F, 0.F, 1.F, 1.F };
		}

		engine::color gray() {
			return engine::color { .5F, .5F, .5F, 1.F };
		}

		PaintStyleAttributes storedPaint() {
			return PaintStyleAttributes { red(), math::fvec4 { 1.F, 1.F, 1.F, 1.F } };
		}

		theming::StateLayer hoverTint(const InteractiveStateFlagBits bit_, ref<const engine::color> tint_) {
			theming::StateLayer layer { .mask = StyleStates { bit_ } };
			layer.paint.with<attr::PaintStyle::tint>(tint_);
			return layer;
		}

		theming::ComponentStyle withLayers(mref<Vector<theming::StateLayer>> layers_) {
			return theming::ComponentStyle { .states = ::hg::move(layers_) };
		}

		void resolvePaint(
			ref<const theming::Theme> theme_,
			const StyleStates states_,
			ref<const theming::StyleRule<PaintStyleAttributes>> instance_,
			ref<PaintStyleAttributes> inOut_
		) {
			theming::ThemeContext context {};
			std::ignore = context.addLocalTheme(std::addressof(theme_));
			theming::resolveStyle<PaintStyleAttributes, attr::PaintStyle::tint, attr::PaintStyle::borderRadius>(
				context,
				ReflowClassList { "[Btn]"sv },
				states_,
				instance_,
				inOut_
			);
		}
	}
}
