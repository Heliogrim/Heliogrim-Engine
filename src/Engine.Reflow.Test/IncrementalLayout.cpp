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
		/**
		 * Stands in for wrapping text: no font is loaded in the test, so Text would measure zero.
		 */
		class WrapProbe final :
			public uikit::Paint {
		public:
			PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override {
				auto sizing = Paint::prefetchSizing(axis_, state_);
				if (axis_ == ReflowAxis::eXAxis) {
					sizing.sizing.x = 400.F;
					return sizing;
				}
				const auto width = std::max(getLayoutState().computeSize.x, 1.F);
				sizing.sizing.y = std::ceil(400.F / width) * 10.F;
				return sizing;
			}
		};

		struct Params {
			float leftMinWidth = 40.F;
			float wrapWidth = 300.F;
			bool fixedLeft = false;
		};

		struct Scene {
			sptr<uikit::VerticalLayout> root;
			sptr<uikit::HorizontalLayout> rowA;
			sptr<uikit::HorizontalLayout> rowB;
			sptr<uikit::BoxLayout> leftBox;
			sptr<ProbePaint> leftLeaf;
			sptr<uikit::VerticalLayout> sibling;
			sptr<ProbePaint> rightLeaf;
			sptr<uikit::BoxLayout> wrapBox;
			sptr<WrapProbe> wrapText;
		};

		[[nodiscard]] ReflowUnit abs(const float value_) {
			return ReflowUnit { .type = ReflowUnitType::eAbsolute, .value = value_ };
		}

		[[nodiscard]] Scene makeScene(const Params p_) {
			Scene s {};
			s.root = make_sptr<uikit::VerticalLayout>();
			s.rowA = make_sptr<uikit::HorizontalLayout>();
			s.rowB = make_sptr<uikit::HorizontalLayout>();
			s.root->addChild(s.rowA);
			s.root->addChild(s.rowB);

			s.leftBox = make_sptr<uikit::BoxLayout>();
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(p_.leftMinWidth));
			if (p_.fixedLeft) {
				s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(p_.leftMinWidth));
				s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(50.F));
				s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(abs(50.F));
			}
			s.leftLeaf = make_sptr<ProbePaint>();
			s.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(
				ReflowUnit { .type = ReflowUnitType::eRelative, .value = 0.5F }
			);
			s.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(10.F));
			s.leftBox->setChild(s.leftLeaf);
			s.rowA->addChild(s.leftBox);

			s.sibling = make_sptr<uikit::VerticalLayout>();
			auto upper = make_sptr<ProbePaint>();
			upper->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(20.F));
			upper->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(10.F));
			s.sibling->addChild(upper);
			s.rightLeaf = make_sptr<ProbePaint>();
			s.rightLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(20.F));
			s.rightLeaf->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(10.F));
			s.sibling->addChild(s.rightLeaf);
			s.rowA->addChild(s.sibling);

			s.wrapBox = make_sptr<uikit::BoxLayout>();
			s.wrapBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(p_.wrapWidth));
			s.wrapText = make_sptr<WrapProbe>();
			s.wrapBox->setChild(s.wrapText);
			s.rowB->addChild(s.wrapBox);
			return s;
		}

		void expectSameLayout(ref<const Widget> lhs_, ref<const Widget> rhs_) {
			EXPECT_EQ(lhs_.getLayoutState().layoutOffset, rhs_.getLayoutState().layoutOffset) << lhs_.getTag();
			EXPECT_EQ(lhs_.getLayoutState().layoutSize, rhs_.getLayoutState().layoutSize) << lhs_.getTag();

			const auto& lc = *lhs_.children();
			const auto& rc = *rhs_.children();
			auto li = lc.begin();
			auto ri = rc.begin();
			for (; li != lc.end() && ri != rc.end(); ++li, ++ri) {
				expectSameLayout(**li, **ri);
			}
			EXPECT_TRUE(li == lc.end()) << lhs_.getTag();
			EXPECT_TRUE(ri == rc.end()) << lhs_.getTag();
		}
	}

	class IncrementalLayoutFixture :
		public TreeFixture {
	public:
		Scene s;

	public:
		void SetUp() override {
			s = makeScene({});
		}

		void TearDown() override {
			TreeFixture::TearDown();
			s = {};
		}

		[[nodiscard]] static Scene fresh(const Params p_, const math::vec2 size_ = math::vec2 { 320.F, 240.F }) {
			auto other = makeScene(p_);
			ReflowState local {};
			ReflowEngine::tick(
				local,
				other.root,
				LayoutContext { .localOffset = math::vec2 { 0.F }, .localSize = size_, .localScale = 1.F }
			);
			local.reset();
			return other;
		}
	};

	TEST_F(IncrementalLayoutFixture, UnchangedFrameLaysOutNoWidget) {
		tick(s.root);
		const auto before = state.layoutCalls();
		tick(s.root);

		EXPECT_EQ(state.layoutCalls(), before);
	}

	TEST_F(IncrementalLayoutFixture, UnrecordedWidgetReportsOwnLayoutState) {
		const sptr<Widget> unrecorded = make_sptr<ProbePaint>();
		const auto* const expected = std::addressof(unrecorded->getLayoutState());

		const auto viewed = std::as_const(state).getStateOf(unrecorded);
		const auto mutated = state.getStateOf(unrecorded);

		ASSERT_NE(viewed.get(), nullptr);
		EXPECT_EQ(viewed.get(), expected);
		ASSERT_NE(mutated.get(), nullptr);
		EXPECT_EQ(mutated.get(), expected);
	}

	TEST_F(IncrementalLayoutFixture, PaintOnlyChangeLaysOutNoWidget) {
		tick(s.root);
		const auto before = state.layoutCalls();

		s.leftLeaf->getStyleAttributes().update<attr::PaintStyle::tint>(engine::color { 1.F, 0.F, 0.F, 1.F });
		state.setRenderTick(7);
		tick(s.root);

		EXPECT_EQ(state.layoutCalls(), before);
		EXPECT_TRUE(s.leftLeaf->state().shouldRenderSelf(7));
	}

	TEST_F(IncrementalLayoutFixture, SiblingChangeSkipsUnmovedSubtree) {
		tick(s.root);
		s.leftLeaf->prefetches = 0u;
		s.leftLeaf->computes = 0u;

		s.rightLeaf->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(30.F));
		tick(s.root);

		EXPECT_EQ(s.leftLeaf->prefetches + s.leftLeaf->computes, 0u);
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_SiblingMoves) {
		tick(s.root);
		const auto offsetBefore = s.rightLeaf->getLayoutState().layoutOffset;

		s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(80.F));
		tick(s.root);

		const auto other = fresh({ .leftMinWidth = 80.F });
		expectSameLayout(*s.root, *other.root);
		EXPECT_NE(s.rightLeaf->getLayoutState().layoutOffset, offsetBefore);
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_WrapNarrows) {
		tick(s.root);
		const auto heightBefore = s.wrapText->getLayoutState().layoutSize.y;

		s.wrapBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(80.F));
		tick(s.root);

		const auto other = fresh({ .wrapWidth = 80.F });
		expectSameLayout(*s.root, *other.root);
		EXPECT_GT(s.wrapText->getLayoutState().layoutSize.y, heightBefore);
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_Resize) {
		tick(s.root);

		ctx.localSize = math::vec2 { 400.F, 300.F };
		tick(s.root);

		const auto other = fresh({}, math::vec2 { 400.F, 300.F });
		expectSameLayout(*s.root, *other.root);
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_ReplaceChild) {
		tick(s.root);

		s.leftBox->setChild(make_sptr<ProbePaint>());
		tick(s.root);

		auto other = makeScene({});
		other.leftBox->setChild(make_sptr<ProbePaint>());
		ReflowState local {};
		ReflowEngine::tick(local, other.root, clone(ctx));
		expectSameLayout(*s.root, *other.root);
	}

	TEST_F(IncrementalLayoutFixture, TickerInCleanSubtreeStillTicks) {
		s.rightLeaf->ticking = true;

		tick(s.root);
		const auto before = state.layoutCalls();
		tick(s.root);
		tick(s.root);

		EXPECT_EQ(s.rightLeaf->ticks, 3u);
		EXPECT_EQ(state.layoutCalls(), before);
	}

	TEST_F(IncrementalLayoutFixture, FixedBoxStopsLayoutPropagation) {
		s = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		tick(s.root);

		s.leftLeaf->markLayoutPending();

		EXPECT_TRUE(s.leftBox->state().isLayoutPending());
		EXPECT_TRUE(s.rowA->state().isSubtreePending());
		EXPECT_FALSE(s.rowA->state().isLayoutPending());
		EXPECT_TRUE(s.root->state().isSubtreePending());
		EXPECT_FALSE(s.root->state().isLayoutPending());
	}

	TEST_F(IncrementalLayoutFixture, PendingBoundaryResizeRelaysParent) {
		s = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		tick(s.root);
		const auto offsetBefore = s.rightLeaf->getLayoutState().layoutOffset;

		s.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(30.F));
		s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(120.F));
		s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(120.F));
		tick(s.root);

		auto other = makeScene({ .leftMinWidth = 120.F, .fixedLeft = true });
		other.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(30.F));
		ReflowState local {};
		ReflowEngine::tick(local, other.root, clone(ctx));
		expectSameLayout(*s.root, *other.root);
		EXPECT_NE(s.rightLeaf->getLayoutState().layoutOffset, offsetBefore);
	}

	TEST_F(IncrementalLayoutFixture, FixedBoxChangeSkipsAncestors) {
		s = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		tick(s.root);
		s.rightLeaf->prefetches = 0u;
		s.rightLeaf->computes = 0u;
		const auto before = state.layoutCalls();

		s.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(30.F));
		state.setRenderTick(13);
		tick(s.root);

		EXPECT_EQ(s.rightLeaf->prefetches + s.rightLeaf->computes, 0u);
		// Prefetch and compute count per axis: (2 + 2 + 1) calls for each of leftBox and leftLeaf
		EXPECT_EQ(state.layoutCalls() - before, 10u);
		EXPECT_TRUE(s.leftBox->state().shouldRenderSelf(13));
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_Boundary) {
		s = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		tick(s.root);

		s.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(30.F));
		tick(s.root);

		auto other = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		other.leftLeaf->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(30.F));
		ReflowState local {};
		ReflowEngine::tick(local, other.root, clone(ctx));
		expectSameLayout(*s.root, *other.root);
	}

	TEST_F(IncrementalLayoutFixture, AutoBoxIsNotBoundary) {
		EXPECT_FALSE(uikit::BoxLayout {}.isLayoutBoundary());
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_BoundaryResizes) {
		s = makeScene({ .leftMinWidth = 100.F, .fixedLeft = true });
		tick(s.root);
		const auto offsetBefore = s.rightLeaf->getLayoutState().layoutOffset;

		{
			AttributeBatch batch { *s.leftBox };
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(120.F));
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(120.F));
		}
		tick(s.root);

		const auto other = fresh({ .leftMinWidth = 120.F, .fixedLeft = true });
		expectSameLayout(*s.root, *other.root);
		EXPECT_NE(s.rightLeaf->getLayoutState().layoutOffset, offsetBefore);
	}

	TEST_F(IncrementalLayoutFixture, IncrementalEqualsFull_AutoToFixed) {
		tick(s.root);

		{
			AttributeBatch batch { *s.leftBox };
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>(abs(100.F));
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>(abs(100.F));
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>(abs(50.F));
			s.leftBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>(abs(50.F));
		}
		tick(s.root);

		const auto other = fresh({ .leftMinWidth = 100.F, .fixedLeft = true });
		expectSameLayout(*s.root, *other.root);
	}
}
