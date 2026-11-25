#include "pch.h"

/**/

#include "RouterFixture.hpp"

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	namespace {
		[[nodiscard]] ReflowUnit abs(const float value_) {
			return ReflowUnit { ReflowUnitType::eAbsolute, value_ };
		}

		[[nodiscard]] engine::color baseTint() {
			return engine::color { 0.2F, 0.2F, 0.2F, 1.F };
		}

		[[nodiscard]] engine::color focusTint() {
			return engine::color { 0.F, 0.5F, 1.F, 1.F };
		}

		[[nodiscard]] math::fvec4 invalidRadius() {
			return math::fvec4 { 3.F, 3.F, 3.F, 3.F };
		}

		[[nodiscard]] Vector<theming::Theme> fieldTheme() {
			theming::ComponentStyle field {};
			field.paint.with<attr::PaintStyle::tint>(baseTint());

			theming::StateLayer focus { .mask = StyleStates { InteractiveStateFlagBits::eFocus } };
			focus.paint.with<attr::PaintStyle::tint>(focusTint());
			theming::StateLayer invalid { .mask = StyleStates { InteractiveStateFlagBits::eInvalid } };
			invalid.paint.with<attr::PaintStyle::borderRadius>(invalidRadius());
			field.states.emplace_back(::hg::move(focus));
			field.states.emplace_back(::hg::move(invalid));

			theming::Theme theme {};
			theme.addComponentStyle("field"sv, ::hg::move(field));

			Vector<theming::Theme> themes {};
			themes.emplace_back(::hg::move(theme));
			return themes;
		}

		[[nodiscard]] uikit::Desc describedTree() {
			return uikit::desc::Column { .padding = Padding { 4.F }, .colGap = 2.F, .name = "root" }(
				uikit::desc::Label { .minHeight = abs(12.F), .text = String { "a" }, .name = "label" },
				uikit::desc::Surface { .minHeight = abs(20.F), .tint = focusTint(), .name = "field", .component = "field" }
			);
		}

		[[nodiscard]] sptr<Widget> imperativeTree() {
			auto label = make_sptr<uikit::Text>();
			label->set<attr::BoxLayout::minHeight>(abs(12.F));
			label->set<attr::TextData::text>(String { "a" });
			label->setName("label"sv);

			auto surface = make_sptr<uikit::Surface>();
			surface->set<attr::BoxLayout::minHeight>(abs(20.F));
			surface->set<attr::PaintStyle::tint>(focusTint());
			surface->instance.with<attr::PaintStyle::tint>(focusTint());
			surface->component = "field"sv;
			surface->setName("field"sv);

			auto column = make_sptr<uikit::VerticalLayout>();
			column->set<attr::BoxLayout::padding>(Padding { 4.F });
			column->set<attr::FlexLayout::colGap>(2.F);
			column->setName("root"sv);
			column->addChild(label);
			column->addChild(surface);
			return column;
		}

		void expectUnit(ref<const ReflowUnit> left_, ref<const ReflowUnit> right_) {
			EXPECT_EQ(left_.type, right_.type);
			EXPECT_EQ(left_.value, right_.value);
		}

		template <class Atom_>
		void expectBox(ref<const Atom_> left_, ref<const Atom_> right_) {
			expectUnit(left_.template get<attr::BoxLayout::minWidth>(), right_.template get<attr::BoxLayout::minWidth>());
			expectUnit(left_.template get<attr::BoxLayout::maxWidth>(), right_.template get<attr::BoxLayout::maxWidth>());
			expectUnit(left_.template get<attr::BoxLayout::minHeight>(), right_.template get<attr::BoxLayout::minHeight>());
			expectUnit(left_.template get<attr::BoxLayout::maxHeight>(), right_.template get<attr::BoxLayout::maxHeight>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::widthGrow>(), right_.template get<attr::BoxLayout::widthGrow>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::widthShrink>(), right_.template get<attr::BoxLayout::widthShrink>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::heightGrow>(), right_.template get<attr::BoxLayout::heightGrow>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::heightShrink>(), right_.template get<attr::BoxLayout::heightShrink>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::padding>(), right_.template get<attr::BoxLayout::padding>());
			EXPECT_EQ(left_.template get<attr::BoxLayout::placement>(), right_.template get<attr::BoxLayout::placement>());
		}
	}

	TEST(Description, MountBuildsTreeAndSetsSlots) {
		const auto root = uikit::mount(
			uikit::desc::Column { .padding = Padding { 4.F } }(
				uikit::desc::Label { .text = String { "a" } },
				uikit::desc::Surface { .minHeight = abs(20.F) }
			)
		);

		const auto column = std::dynamic_pointer_cast<uikit::VerticalLayout>(root);
		ASSERT_NE(column, nullptr);
		EXPECT_EQ(column->get<attr::BoxLayout::padding>(), Padding { 4.F });
		ASSERT_EQ(column->children()->size(), 2uLL);

		const auto label = std::dynamic_pointer_cast<uikit::Text>(column->children()->front());
		ASSERT_NE(label, nullptr);
		EXPECT_EQ(label->get<attr::TextData::text>(), String { "a" });

		const auto surface = std::dynamic_pointer_cast<uikit::Surface>(column->children()->back());
		ASSERT_NE(surface, nullptr);
		expectUnit(surface->get<attr::BoxLayout::minHeight>(), abs(20.F));
	}

	TEST(Description, RefAndFind) {
		sptr<uikit::Surface> field = nullptr;
		const auto root = uikit::mount(
			uikit::desc::Column {}(uikit::desc::Label {}, uikit::desc::Surface { .name = "field", .ref = &field })
		);

		ASSERT_NE(field, nullptr);
		EXPECT_EQ(uikit::find<uikit::Paint>(root, "field"sv), field);
		EXPECT_EQ(uikit::find<uikit::Text>(root, "field"sv), nullptr);
		EXPECT_EQ(uikit::find<uikit::Paint>(root, "missing"sv), nullptr);
	}

	TEST_F(RouterFixture, DescriptionSurfaceShowsCombinedStates) {
		sptr<uikit::Surface> field = nullptr;
		auto provider = make_sptr<uikit::ThemeProvider>();
		provider->setChild(uikit::mount(uikit::desc::Surface { .minWidth = abs(50.F), .minHeight = abs(20.F), .ref = &field, .component = "field" }));
		provider->setProviderThemes(fieldTheme());
		mount(provider);
		provider->cascadeContextChange(false);
		ASSERT_NE(field, nullptr);

		const auto& style = field->getStyleAttributes();
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), baseTint());

		router.focus(window, field);
		field->setStyleState(InteractiveStateFlagBits::eInvalid, true);
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), focusTint());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), invalidRadius());

		router.blur(window);
		EXPECT_FALSE(field->styleStates() & InteractiveStateFlagBits::eFocus);
		EXPECT_EQ(style.valueOf<attr::PaintStyle::tint>(), baseTint());
		EXPECT_EQ(style.valueOf<attr::PaintStyle::borderRadius>(), invalidRadius());
	}

	TEST(Description, SurfaceIgnoresLegacyClassThemes) {
		const engine::color legacyTint { 0.F, 1.F, 0.F, 1.F };
		const math::fvec4 legacyRadius { 7.F, 7.F, 7.F, 7.F };
		theming::Theme theme {};
		theme.addPaintStyle("[Paint]"sv, PaintStyleAttributes { legacyTint, legacyRadius });
		theme.addPaintStyle("[Surface]"sv, PaintStyleAttributes { legacyTint, legacyRadius });
		Vector<theming::Theme> themes {};
		themes.emplace_back(::hg::move(theme));

		sptr<uikit::Surface> field = nullptr;
		auto column = uikit::mount(
			uikit::desc::Column {}(uikit::desc::Surface { .tint = focusTint(), .ref = &field }, uikit::desc::Label {})
		);
		auto paint = make_sptr<uikit::Paint>();
		std::static_pointer_cast<uikit::VerticalLayout>(column)->addChild(paint);

		auto provider = make_sptr<uikit::ThemeProvider>();
		provider->setChild(column);
		provider->setProviderThemes(::hg::move(themes));
		provider->cascadeContextChange(false);
		ASSERT_NE(field, nullptr);

		// The plain paint proves the legacy theme reaches this subtree.
		EXPECT_EQ(paint->get<attr::PaintStyle::tint>(), legacyTint);
		EXPECT_EQ(field->get<attr::PaintStyle::tint>(), focusTint());
		EXPECT_EQ(field->get<attr::PaintStyle::borderRadius>(), PaintStyleAttributes {}.valueOf<attr::PaintStyle::borderRadius>());
	}

	TEST(Description, MatchesImperativeBuild) {
		const auto described = uikit::mount(describedTree());
		const auto imperative = imperativeTree();

		const auto leftColumn = std::dynamic_pointer_cast<uikit::VerticalLayout>(described);
		const auto rightColumn = std::dynamic_pointer_cast<uikit::VerticalLayout>(imperative);
		ASSERT_NE(leftColumn, nullptr);
		ASSERT_NE(rightColumn, nullptr);
		expectBox(*leftColumn, *rightColumn);
		EXPECT_EQ(leftColumn->get<attr::FlexLayout::colGap>(), rightColumn->get<attr::FlexLayout::colGap>());
		EXPECT_EQ(leftColumn->get<attr::FlexLayout::rowGap>(), rightColumn->get<attr::FlexLayout::rowGap>());
		EXPECT_EQ(leftColumn->get<attr::FlexLayout::align>(), rightColumn->get<attr::FlexLayout::align>());
		EXPECT_EQ(leftColumn->get<attr::FlexLayout::justify>(), rightColumn->get<attr::FlexLayout::justify>());
		EXPECT_EQ(described->getName(), imperative->getName());
		ASSERT_EQ(leftColumn->children()->size(), rightColumn->children()->size());

		const auto leftLabel = std::dynamic_pointer_cast<uikit::Text>(leftColumn->children()->front());
		const auto rightLabel = std::dynamic_pointer_cast<uikit::Text>(rightColumn->children()->front());
		ASSERT_NE(leftLabel, nullptr);
		ASSERT_NE(rightLabel, nullptr);
		expectBox(*leftLabel, *rightLabel);
		EXPECT_EQ(leftLabel->get<attr::TextData::text>(), rightLabel->get<attr::TextData::text>());
		EXPECT_EQ(leftLabel->get<attr::TextStyle::fontSize>(), rightLabel->get<attr::TextStyle::fontSize>());
		EXPECT_EQ(leftLabel->get<attr::TextStyle::color>(), rightLabel->get<attr::TextStyle::color>());
		EXPECT_EQ(leftLabel->getName(), rightLabel->getName());

		const auto leftSurface = std::dynamic_pointer_cast<uikit::Surface>(leftColumn->children()->back());
		const auto rightSurface = std::dynamic_pointer_cast<uikit::Surface>(rightColumn->children()->back());
		ASSERT_NE(leftSurface, nullptr);
		ASSERT_NE(rightSurface, nullptr);
		expectBox(*leftSurface, *rightSurface);
		EXPECT_EQ(leftSurface->get<attr::PaintStyle::tint>(), rightSurface->get<attr::PaintStyle::tint>());
		EXPECT_EQ(leftSurface->get<attr::PaintStyle::borderRadius>(), rightSurface->get<attr::PaintStyle::borderRadius>());
		EXPECT_EQ(leftSurface->component, rightSurface->component);
		EXPECT_EQ(leftSurface->getName(), rightSurface->getName());
	}
}
