#include "pch.h"

/**/

#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>

#include "TreeFixture.hpp"

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	class RenderVersionFixture :
		public TreeFixture {
	public:
		sptr<uikit::VerticalLayout> root;
		sptr<uikit::VerticalLayout> mid;
		sptr<ProbePaint> leaf;
		sptr<ProbePaint> other;

	public:
		void SetUp() override {
			root = make_sptr<uikit::VerticalLayout>();
			mid = make_sptr<uikit::VerticalLayout>();
			leaf = make_sptr<ProbePaint>();
			other = make_sptr<ProbePaint>();
			root->addChild(mid);
			mid->addChild(leaf);
			root->addChild(other);
		}

		void TearDown() override {
			TreeFixture::TearDown();
			other.reset();
			leaf.reset();
			mid.reset();
			root.reset();
		}

		void wrapWithStaleVersions(const u16 stale_) {
			state.setRenderTick(3u);
			tick(root);

			leaf->state().setRenderVersion(stale_);
			leaf->state().setProxyRenderVersion(stale_);
			mid->state().setProxyRenderVersion(stale_);

			// Only the sibling is pending, so the layout passes never visit the stale subtree
			other->markPaintPending();
			state.setRenderTick(0u);
			tick(root);
		}
	};

	TEST_F(RenderVersionFixture, WrapResetsCleanSubtree) {
		wrapWithStaleVersions(7u);

		EXPECT_FALSE(leaf->state().shouldRenderSelf(1u));
		EXPECT_FALSE(leaf->state().shouldProxyRender(1u));
		EXPECT_FALSE(mid->state().shouldProxyRender(1u));
	}

	TEST_F(RenderVersionFixture, PropagatesToRoot) {
		leaf->updateRenderVersion(5);

		EXPECT_EQ(leaf->state().shouldRenderSelf(5), true);
		EXPECT_EQ(mid->state().proxyRenderVersion(), 5);
		EXPECT_EQ(root->state().proxyRenderVersion(), 5);
	}

	TEST_F(RenderVersionFixture, StopsAtInheritedAncestor) {
		mid->state().setProxyRenderVersion(5);

		leaf->updateRenderVersion(5);

		EXPECT_TRUE(leaf->state().shouldRenderSelf(5));
		EXPECT_EQ(root->state().proxyRenderVersion(), 0);
	}

	TEST_F(RenderVersionFixture, EarlyExitAcrossWrap) {
		leaf->updateRenderVersion(0xFFFF);
		leaf->updateRenderVersion(0);

		EXPECT_EQ(root->state().proxyRenderVersion(), 0);
	}

	TEST_F(RenderVersionFixture, SelfVersionDoesNotStopProxy) {
		mid->state().setRenderVersion(5);

		leaf->updateRenderVersion(5);

		EXPECT_EQ(root->state().proxyRenderVersion(), 5);
	}

	TEST_F(RenderVersionFixture, UpdateAfterWrapReachesRoot) {
		wrapWithStaleVersions(1u);

		leaf->updateRenderVersion(1u);

		EXPECT_EQ(mid->state().proxyRenderVersion(), 1u);
		EXPECT_EQ(root->state().proxyRenderVersion(), 1u);
	}
}
