#include "pch.h"

/**/

#include "RouterFixture.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	namespace {
		constexpr char escape = 0x1B;

		[[nodiscard]] sptr<RouterFixture::Leaf> popupLeaf(string name_, const ptr<Vector<string>> log_) {
			auto leaf = make_sptr<RouterFixture::Leaf>(std::move(name_), log_);
			RouterFixture::size(leaf, 40.F);
			leaf->focusable = true;
			return leaf;
		}

		[[nodiscard]] PopupAnchor below(cref<sptr<Widget>> widget_) {
			const auto& layout = widget_->getLayoutState();
			return PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart };
		}
	}

	TEST_F(RouterFixture, PopupFocusMovesInAndRestores) {
		c->focusable = true;
		mouse(5, 5, true);
		mouse(5, 5, false);
		ASSERT_EQ(window->getFocusTarget(), c);

		bool closed = false;
		const auto p = popupLeaf("p", &log);
		const auto handle = router.openPopup(window, p, below(c), c, None, [&closed]() { closed = true; });

		EXPECT_TRUE(router.hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), p);
		EXPECT_NE(nearestScope(p), nullptr);
		EXPECT_EQ(nearestScope(p)->kind(), FocusScopeKind::ePopup);

		router.closePopup(handle);

		EXPECT_TRUE(closed);
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), c);
	}

	TEST_F(RouterFixture, PopupOutsideClickClosesAndIsConsumed) {
		bool closed = false;
		const auto p = popupLeaf("p", &log);
		discard = router.openPopup(window, p, below(c), c, None, [&closed]() { closed = true; });
		tick(window);

		// Inside the popup: stays open and reaches the popup content.
		mouse(5, 25, true);
		mouse(5, 25, false);
		EXPECT_FALSE(closed);
		EXPECT_EQ(filtered("down:p").size(), 1uLL);

		log.clear();
		const auto reply = mouse(150, 150, true);
		mouse(150, 150, false);

		EXPECT_TRUE(closed);
		EXPECT_EQ(reply.response, EventResponse::eConsumed);
		EXPECT_TRUE(filtered("down:").empty());
		EXPECT_TRUE(filtered("preview-down:").empty());
		// The release of the dismissing press does not reach (click) what lies below.
		EXPECT_TRUE(filtered("up:").empty());
	}

	TEST_F(RouterFixture, PopupAnchorClickClosesAndIsConsumed) {
		bool closed = false;
		discard = router.openPopup(window, popupLeaf("p", &log), below(c), c, None, [&closed]() { closed = true; });
		tick(window);

		const auto reply = mouse(5, 5, true);

		EXPECT_TRUE(closed);
		EXPECT_EQ(reply.response, EventResponse::eConsumed);
		EXPECT_TRUE(filtered("down:c").empty());

		mouse(5, 5, false);
		EXPECT_TRUE(filtered("up:").empty());

		// The next press is an ordinary one again.
		mouse(5, 5, true);
		mouse(5, 5, false);
		EXPECT_EQ(filtered("down:c").size(), 1uLL);
		EXPECT_EQ(filtered("up:c").size(), 1uLL);
	}

	TEST_F(RouterFixture, PopupCoveringItsAnchorKeepsInsideDowns) {
		// A select flipped over its own field: the popup lies on top of the anchor widget.
		bool closed = false;
		const auto p = popupLeaf("p", &log);
		const auto anchor = PopupAnchor { .offset = math::vec2 { 0.F }, .size = math::vec2 { 0.F }, .side = PopupSide::eBelow, .align = PopupAlign::eStart };
		discard = router.openPopup(window, p, anchor, c, None, [&closed]() { closed = true; });
		tick(window);
		move(1.F, 1.F);
		tick(window);

		const auto reply = mouse(5, 5, true);
		mouse(5, 5, false);

		EXPECT_FALSE(closed);
		EXPECT_FALSE(reply.isConsumed());
		EXPECT_EQ(filtered("down:p").size(), 1uLL);
		EXPECT_TRUE(filtered("down:c").empty());
	}

	TEST_F(RouterFixture, PopupEscapeClosesTopOnly) {
		bool parentClosed = false;
		bool childClosed = false;
		const auto parent = popupLeaf("parent", &log);
		const auto child = popupLeaf("child", &log);

		const auto parentHandle = router.openPopup(window, parent, below(c), c, None, [&]() { parentClosed = true; });
		tick(window);
		discard = router.openPopup(window, child, below(parent), parent, Opt<PopupHandle> { parentHandle }, [&]() { childClosed = true; });
		ASSERT_EQ(window->getFocusTarget(), child);

		key(escape);

		EXPECT_TRUE(childClosed);
		EXPECT_FALSE(parentClosed);
		EXPECT_TRUE(router.hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), parent);

		key(escape);

		EXPECT_TRUE(parentClosed);
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
	}

	TEST_F(RouterFixture, PopupParentCloseClosesChildren) {
		c->focusable = true;
		router.focus(window, c);

		Vector<string> order {};
		const auto parent = popupLeaf("parent", &log);
		const auto parentHandle = router.openPopup(window, parent, below(c), c, None, [&]() { order.emplace_back("parent"); });
		tick(window);
		discard = router.openPopup(
			window,
			popupLeaf("child", &log),
			below(parent),
			parent,
			Opt<PopupHandle> { parentHandle },
			[&]() { order.emplace_back("child"); }
		);

		router.closePopup(parentHandle);

		EXPECT_EQ(order, (Vector<string> { "child", "parent" }));
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
		EXPECT_TRUE(window->layers().empty());
		EXPECT_EQ(window->getFocusTarget(), c);
	}

	TEST_F(RouterFixture, ChildCloseDoesNotOverrideParentRestore) {
		c->focusable = true;
		router.focus(window, c);

		const auto parent = popupLeaf("parent", &log);
		const auto parentHandle = router.openPopup(window, parent, below(c), c, None, []() {});
		tick(window);
		// No explicit parent: derived from the anchor sitting inside the parent popup.
		const auto childHandle = router.openPopup(window, popupLeaf("child", &log), below(parent), parent, None, []() {});

		router.closePopup(parentHandle);
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), c);

		// A late close of the child (e.g. from its anchor's destructor) is a no-op.
		router.closePopup(childHandle);
		EXPECT_EQ(window->getFocusTarget(), c);
	}

	TEST_F(RouterFixture, PopupCloseOfClosesChain) {
		c->focusable = true;
		router.focus(window, c);

		Vector<string> order {};
		const auto parent = popupLeaf("parent", &log);
		discard = router.openPopup(window, parent, below(c), c, None, [&]() { order.emplace_back("parent"); });
		tick(window);
		const auto child = popupLeaf("child", &log);
		discard = router.openPopup(window, child, below(parent), parent, None, [&]() { order.emplace_back("child"); });

		EXPECT_TRUE(router.closePopupOf(child));
		EXPECT_EQ(order, (Vector<string> { "child", "parent" }));
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_FALSE(router.closePopupOf(c));
	}

	TEST_F(RouterFixture, PopupAnchorUnmountCloses) {
		bool closed = false;
		discard = router.openPopup(window, popupLeaf("p", &log), below(c), c, None, [&closed]() { closed = true; });

		mount(make_sptr<Leaf>("other", &log));
		EXPECT_FALSE(closed);

		move(1.F, 1.F);

		EXPECT_TRUE(closed);
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
	}

	TEST_F(RouterFixture, PopupWindowFocusLossCloses) {
		bool closed = false;
		discard = router.openPopup(window, popupLeaf("p", &log), below(c), c, None, [&closed]() { closed = true; });

		const auto other = make_sptr<Window>();
		other->setClientSize(math::vec2 { 100.F, 100.F });
		other->setContent(make_sptr<Leaf>("other", &log));
		router.registerWindow(other);
		tick(other);

		router.focus(other, other);

		EXPECT_TRUE(closed);
		EXPECT_FALSE(router.hasOpenPopup(window.get()));
		router.unregisterWindow(other.get());
	}

	TEST_F(RouterFixture, PopupPlacedInsideClient) {
		const auto anchor = PopupAnchor {
			.offset = math::vec2 { 300.F, 230.F }, .size = math::vec2 { 20.F, 10.F }, .side = PopupSide::eBelow, .align = PopupAlign::eStart
		};
		const auto handle = router.openPopup(window, popupLeaf("p", &log), anchor, c, None, []() {});
		tick(window);
		move(1.F, 1.F);
		tick(window);

		ASSERT_EQ(window->layers().size(), 1uLL);
		EXPECT_EQ(window->layers().front()->band, LayerBand::ePopup);
		const auto host = window->layers().front()->host.lock();
		ASSERT_NE(host, nullptr);

		const auto& layout = host->getLayoutState();
		EXPECT_EQ(layout.layoutSize.x, 40.F);
		EXPECT_EQ(layout.layoutSize.y, 40.F);
		EXPECT_GE(layout.layoutOffset.x, 0.F);
		EXPECT_GE(layout.layoutOffset.y, 0.F);
		EXPECT_LE(layout.layoutOffset.x + layout.layoutSize.x, 320.F);
		EXPECT_LE(layout.layoutOffset.y + layout.layoutSize.y, 240.F);

		// Closing drops the layer and detaches its host from the window.
		router.closePopup(handle);
		EXPECT_TRUE(window->layers().empty());
		EXPECT_EQ(window->children()->size(), 2uLL);
		EXPECT_EQ(host->parent(), nullptr);
	}

	TEST_F(RouterFixture, HostOffsetMovesLayerContent) {
		const auto popup = make_sptr<Leaf>("popup", &log);
		size(popup, 20.F);
		const auto host = make_sptr<Host>(popup, math::vec2 { 0.F }, math::vec2 { 320.F, 240.F });
		[[maybe_unused]] const auto layer = window->requestLayer(LayerBand::ePopup, host);
		tick(window);

		host->setHostOffset(math::vec2 { 250.F, 200.F });
		host->markAsPending(true, true);
		tick(window);

		EXPECT_EQ(popup->getLayoutState().layoutOffset.x, 250.F);
		EXPECT_EQ(popup->getLayoutState().layoutOffset.y, 200.F);
		mouse(255, 205, true);
		EXPECT_EQ(filtered("down"), (Vector<string> { "down:popup" }));
		window->dropLayer(host.get());
	}
}
