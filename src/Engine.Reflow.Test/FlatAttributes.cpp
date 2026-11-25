#include "pch.h"

/**/

#include "TreeFixture.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

static_assert(not uniqueSlotNames<BoxLayoutAttributes, BoxLayoutAttributes>);
static_assert(uniqueSlotNames<BoxLayoutAttributes, FlexLayoutAttributes>);

namespace ReflowModule {
	TEST(FlatAttributes, SetFindsSlotInAnyGroup) {
		auto layout = make_sptr<uikit::VerticalLayout>();

		EXPECT_TRUE(layout->set<attr::FlexLayout::colGap>(4.F + 3.F));
		EXPECT_TRUE(layout->set<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 42.F }));

		EXPECT_EQ(layout->get<attr::FlexLayout::colGap>(), 7.F);
		EXPECT_EQ(layout->get<attr::BoxLayout::minWidth>().type, ReflowUnitType::eAbsolute);
		EXPECT_EQ(layout->get<attr::BoxLayout::minWidth>().value, 42.F);
	}

	class FlatAttributesFixture :
		public TreeFixture {
	public:
		sptr<uikit::VerticalLayout> root;
		sptr<ProbePaint> leaf;

	public:
		void SetUp() override {
			root = make_sptr<uikit::VerticalLayout>();
			leaf = make_sptr<ProbePaint>();
			root->addChild(leaf);
			tick(root);
		}

		void TearDown() override {
			TreeFixture::TearDown();
			leaf.reset();
			root.reset();
		}
	};

	TEST_F(FlatAttributesFixture, SetMarksSlotPhase) {
		ASSERT_FALSE(leaf->state().isAnyPending());

		EXPECT_TRUE(leaf->set<attr::PaintStyle::tint>(engine::color { 0.F, 1.F, 0.F, 1.F }));
		EXPECT_TRUE(leaf->state().isRenderPending());
		EXPECT_FALSE(leaf->state().isLayoutPending());

		EXPECT_TRUE(leaf->set<attr::BoxLayout::padding>(Padding { 9.F }));
		EXPECT_TRUE(leaf->state().isLayoutPending());
	}

	TEST_F(FlatAttributesFixture, SetEqualValueReturnsFalse) {
		const auto padding = leaf->get<attr::BoxLayout::padding>();

		EXPECT_FALSE(leaf->set<attr::BoxLayout::padding>(padding));
		EXPECT_FALSE(leaf->state().isAnyPending());
	}
}
