#include "pch.h"

/**/

#include <Engine.Reflow.Attributes/Style/Base/TextStyleAttributes.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Atom/ThemeProvider.hpp>
#include <Engine.Reflow/Widget/AttributeBatch.hpp>
#include <Engine.Reflow/Widget/NullWidget.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "TreeFixture.hpp"

using namespace hg::engine::reflow;
using namespace hg;

static_assert(
	std::remove_cvref_t<decltype(std::declval<const TextStyleAttributes&>().get<attr::TextStyle::fontSize>())>::phase ==
	ReflowPhase::eLayout
);
static_assert(
	std::remove_cvref_t<decltype(std::declval<const TextStyleAttributes&>().get<attr::TextStyle::color>())>::phase ==
	ReflowPhase::ePaint
);

namespace ReflowModule {
	namespace {
		[[nodiscard]] BoxLayoutAttributes panelBox(const float padding_);

		[[nodiscard]] Vector<theming::Theme> panelThemes(const float padding_, const engine::color tint_);
	}

	class InvalidationFixture :
		public TreeFixture {
	public:
		sptr<uikit::VerticalLayout> root;
		sptr<uikit::VerticalLayout> mid;
		sptr<ProbePaint> leaf;
		sptr<uikit::BoxLayout> box;

	public:
		void SetUp() override {
			root = make_sptr<uikit::VerticalLayout>();
			mid = make_sptr<uikit::VerticalLayout>();
			leaf = make_sptr<ProbePaint>();
			root->addChild(mid);
			mid->addChild(leaf);
			box = make_sptr<uikit::BoxLayout>();
			mid->addChild(box);
			tick(root);
		}

		void TearDown() override {
			TreeFixture::TearDown();
			box.reset();
			leaf.reset();
			mid.reset();
			root.reset();
		}
	};

	TEST_F(InvalidationFixture, StartsClean) {
		EXPECT_FALSE(leaf->state().isAnyPending());
		EXPECT_FALSE(mid->state().isAnyPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, MarkLayoutPropagatesToRoot) {
		leaf->markLayoutPending();

		EXPECT_TRUE(leaf->state().isLayoutPending());
		EXPECT_TRUE(mid->state().isLayoutPending());
		EXPECT_TRUE(root->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, MarkPaintMarksOnlySelf) {
		leaf->markPaintPending();

		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(leaf->state().isLayoutPending());
		EXPECT_TRUE(mid->state().isSubtreePending());
		EXPECT_FALSE(mid->state().isLayoutPending());
		EXPECT_TRUE(root->state().isSubtreePending());
		EXPECT_FALSE(root->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, MarkStopsAtMarkedAncestor) {
		mid->state().set(WidgetStateFlagBits::eLayoutPending);
		leaf->markLayoutPending();

		EXPECT_TRUE(leaf->state().isLayoutPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, PaintMarkStopsAtMarkedAncestor) {
		mid->state().set(WidgetStateFlagBits::eSubtreePending);
		leaf->markPaintPending();

		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, StaleSelfBitStillReachesRoot) {
		auto stack = make_sptr<uikit::Stack>();
		auto deep = make_sptr<ProbePaint>();
		mid->addChild(stack);
		stack->addChild(deep);
		tick(root);
		const auto applies = deep->applies;

		deep->state().set(WidgetStateFlagBits::eLayoutPending);
		deep->markLayoutPending();
		tick(root);

		EXPECT_GT(deep->applies, applies);
		EXPECT_FALSE(deep->state().isAnyPending());
		EXPECT_FALSE(stack->state().isAnyPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, ReparentLaysOutMovedChild) {
		auto host = make_sptr<uikit::Stack>();
		auto a = make_sptr<uikit::VerticalLayout>();
		auto b = make_sptr<uikit::VerticalLayout>();
		auto moved = make_sptr<ProbePaint>();
		host->addChild(a);
		host->addChild(b);
		a->addChild(moved);
		tick(host);
		const auto applies = moved->applies;

		a->removeChild(moved);
		b->addChild(moved);
		state.setRenderTick(11);
		tick(host);

		EXPECT_GT(moved->applies, applies);
		EXPECT_TRUE(moved->state().shouldRenderSelf(11));
	}

	TEST_F(InvalidationFixture, CopiedSetDoesNotMarkSource) {
		auto copy = leaf->getLayoutAttributes();
		copy.update<attr::BoxLayout::minWidth>(ReflowUnit { .type = ReflowUnitType::eAbsolute, .value = 77.F });

		EXPECT_FALSE(leaf->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, SetAssignmentMarksItsPhases) {
		auto style = leaf->getStyleAttributes();
		style.update<attr::PaintStyle::tint>(engine::color { 0.F, 1.F, 0.F, 1.F });
		leaf->getStyleAttributes() = style;

		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(leaf->state().isLayoutPending());

		auto layout = leaf->getLayoutAttributes();
		leaf->getLayoutAttributes() = std::move(layout);

		EXPECT_TRUE(leaf->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, DataSlotUpdateMarksLayout) {
		auto text = make_sptr<uikit::Text>();
		std::ignore = text->clearLayoutPending();
		std::ignore = text->clearRenderPending();

		text->getDataAttributes().update<attr::TextData::text>(string { "changed" });

		EXPECT_TRUE(text->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, DataSetAssignmentMarksLayout) {
		auto text = make_sptr<uikit::Text>();
		std::ignore = text->clearLayoutPending();
		std::ignore = text->clearRenderPending();

		const auto copy = text->getDataAttributes();
		text->getDataAttributes() = copy;

		EXPECT_TRUE(text->state().isLayoutPending());

		std::ignore = text->clearLayoutPending();
		auto moved = text->getDataAttributes();
		text->getDataAttributes() = std::move(moved);

		EXPECT_TRUE(text->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, DataSetSelfAssignmentMarksNothing) {
		auto text = make_sptr<uikit::Text>();
		std::ignore = text->clearLayoutPending();
		std::ignore = text->clearRenderPending();

		auto& set = text->getDataAttributes();
		auto& alias = set;
		set = alias;

		EXPECT_FALSE(text->state().isAnyPending());

		set = std::move(alias);

		EXPECT_FALSE(text->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, ReparentMarksOldAndNewParent) {
		auto other = make_sptr<uikit::VerticalLayout>();
		root->addChild(other);
		tick(root);

		other->addChild(leaf);

		EXPECT_TRUE(mid->state().isLayoutPending());
		EXPECT_TRUE(other->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, NullWidgetNeverMarksForeignTree) {
		auto a = make_sptr<uikit::BoxLayout>();
		auto b = make_sptr<uikit::BoxLayout>();
		tick(a);
		tick(b);

		// Null is shared: attached to B, it must stay inert when A drops it as its default child.
		b->setChild(NullWidget::instance());
		tick(a);
		tick(b);

		a->setChild(make_sptr<ProbePaint>());

		EXPECT_FALSE(b->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, TickClearsEveryFlag) {
		leaf->markPaintPending();
		mid->markLayoutPending();

		tick(root);

		EXPECT_FALSE(leaf->state().isAnyPending());
		EXPECT_FALSE(mid->state().isAnyPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, LayoutSlotUpdateMarksLayout) {
		box->getLayoutAttributes().update<attr::BoxLayout::minWidth>(ReflowUnit { .type = ReflowUnitType::eAbsolute, .value = 40.F });

		EXPECT_TRUE(box->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, PaintSlotUpdateMarksPaintOnly) {
		leaf->getStyleAttributes().update<attr::PaintStyle::tint>(engine::color { 1.F, 0.F, 0.F, 1.F });

		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(leaf->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, EqualUpdateMarksNothing) {
		const auto current = box->getLayoutAttributes().valueOf<attr::BoxLayout::minWidth>();
		box->getLayoutAttributes().update<attr::BoxLayout::minWidth>(current);

		EXPECT_FALSE(box->state().isAnyPending());
		EXPECT_FALSE(mid->state().isAnyPending());
		EXPECT_FALSE(root->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, CompositeSlotMarksLayout) {
		std::get<1>(mid->getLayoutAttributes().attributeSets).update<attr::FlexLayout::rowGap>(6.F);

		EXPECT_TRUE(mid->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, BatchDefersToClose) {
		{
			AttributeBatch batch { *box };
			box->getLayoutAttributes().update<attr::BoxLayout::minWidth>(
				ReflowUnit { .type = ReflowUnitType::eAbsolute, .value = 40.F }
			);
			box->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 3.F });

			EXPECT_FALSE(box->state().isAnyPending());
		}

		EXPECT_TRUE(box->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, EmptyBatchMarksNothing) {
		{
			AttributeBatch batch { *box };
		}

		EXPECT_FALSE(box->state().isAnyPending());
	}

	TEST_F(InvalidationFixture, NestedBatchMarksOnOuterClose) {
		{
			AttributeBatch outer { *box };
			{
				AttributeBatch inner { *box };
				box->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 3.F });
			}
			EXPECT_FALSE(box->state().isAnyPending());
		}

		EXPECT_TRUE(box->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, BatchOfPaintOnlyMarksPaint) {
		{
			AttributeBatch batch { *leaf };
			leaf->getStyleAttributes().update<attr::PaintStyle::tint>(engine::color { 0.F, 1.F, 0.F, 1.F });
		}

		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(leaf->state().isLayoutPending());
	}

	TEST_F(InvalidationFixture, ThemeSwapPaddingMarksLayout) {
		auto provider = make_sptr<uikit::ThemeProvider>();
		auto stack = make_sptr<uikit::Stack>();
		stack->getStaticClassList().add("panel"sv);
		provider->setChild(stack);
		provider->setProviderThemes(panelThemes(4.F, engine::color { 1.F, 1.F, 1.F, 1.F }));
		tick(provider);

		provider->setProviderThemes(panelThemes(8.F, engine::color { 1.F, 1.F, 1.F, 1.F }));

		EXPECT_TRUE(stack->state().isLayoutPending());
		tick(provider);
	}

	TEST_F(InvalidationFixture, ThemeSwapTintMarksPaintOnly) {
		auto provider = make_sptr<uikit::ThemeProvider>();
		auto paint = make_sptr<uikit::Paint>();
		paint->getStaticClassList().add("panel"sv);
		provider->setChild(paint);
		provider->setProviderThemes(panelThemes(4.F, engine::color { 1.F, 1.F, 1.F, 1.F }));
		tick(provider);

		provider->setProviderThemes(panelThemes(4.F, engine::color { 0.F, 0.F, 1.F, 1.F }));

		EXPECT_TRUE(paint->state().isRenderPending());
		EXPECT_FALSE(paint->state().isLayoutPending());
		tick(provider);
	}

	/**/

	namespace {
		BoxLayoutAttributes panelBox(const float padding_) {
			const auto autoUnit = ReflowUnit { .type = ReflowUnitType::eAuto, .value = 0.F };
			return BoxLayoutAttributes {
				autoUnit, autoUnit, 0.F, 1.F,
				autoUnit, autoUnit, 0.F, 1.F,
				Padding { padding_ }, ReflowPlacement::eMiddleCenter
			};
		}

		Vector<theming::Theme> panelThemes(const float padding_, const engine::color tint_) {
			theming::Theme theme {};
			theme.addBoxLayout("panel"sv, panelBox(padding_));
			theme.addPaintStyle("panel"sv, PaintStyleAttributes { tint_, math::fvec4 { 0.F } });

			Vector<theming::Theme> themes {};
			themes.emplace_back(::hg::move(theme));
			return themes;
		}
	}
}
