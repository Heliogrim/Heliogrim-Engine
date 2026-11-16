#include "pch.h"

/**/

#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>
#include <Engine.Reflow/Widget/NullWidget.hpp>

#include "TreeFixture.hpp"

using namespace hg::engine::reflow;
using namespace hg;


namespace ReflowModule {
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
}
