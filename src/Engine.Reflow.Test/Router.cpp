#include "pch.h"

/**/

#include "RouterFixture.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	using Log = Vector<string>;

	TEST_F(RouterFixture, TunnelThenBubble) {
		mouse(5, 5, true);
		EXPECT_EQ(log, (Log { "preview-down:a", "preview-down:b", "preview-down:c", "down:c", "down:b", "down:a" }));
	}

	TEST_F(RouterFixture, PreviewConsumedStopsTarget) {
		b->previewDown = EventReply::consumed();
		mouse(5, 5, true);
		EXPECT_EQ(log, (Log { "preview-down:a", "preview-down:b" }));
	}

	TEST_F(RouterFixture, HoverEnterLeaveOrderAndFlags) {
		move(5.F, 5.F);
		EXPECT_EQ(filtered("enter"), (Log { "enter:a", "enter:b", "enter:c" }));
		EXPECT_TRUE(a->state().isHover());
		EXPECT_TRUE(c->state().isHover());

		move(150.F, 150.F);
		EXPECT_EQ(filtered("leave"), (Log { "leave:c", "leave:b" }));
		EXPECT_TRUE(a->state().isHover());
		EXPECT_FALSE(b->state().isHover());
		EXPECT_FALSE(c->state().isHover());
	}

	TEST_F(RouterFixture, PressSetAndCleared) {
		mouse(5, 5, true);
		EXPECT_TRUE(a->state().isPressed());
		EXPECT_TRUE(b->state().isPressed());
		EXPECT_TRUE(c->state().isPressed());

		mouse(5, 5, false);
		EXPECT_FALSE(a->state().isPressed());
		EXPECT_FALSE(b->state().isPressed());
		EXPECT_FALSE(c->state().isPressed());
	}

	TEST_F(RouterFixture, WheelBubblesFromHit) {
		b->wheel = EventReply::consumed();
		router.route(window, engine::input::event::MouseWheelEvent { math::ivec2 { 5, 5 }, math::vec2 { 0.F, 1.F } });
		EXPECT_EQ(log, (Log { "wheel:c", "wheel:b" }));
	}

	TEST_F(RouterFixture, DropBubblesFromHit) {
		router.route(
			window,
			engine::input::event::DragDropEvent { math::ivec2 { 150, 150 }, engine::input::event::DragDropEventTextPayload { "text" } }
		);
		EXPECT_EQ(log, (Log { "drop:a" }));
	}

	TEST_F(RouterFixture, ReplyReleasesCapture) {
		b->down = EventReply::handled().withCapture();
		b->move = EventReply::consumed().withRelease();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		move(5.F, 5.F);
		EXPECT_EQ(router.captured(), nullptr);
	}

	TEST_F(RouterFixture, PressHeldByOtherButtonStays) {
		mouse(5, 5, true, 1u);
		mouse(150, 150, true, 3u);

		mouse(5, 5, false, 1u);
		EXPECT_FALSE(c->state().isPressed());
		EXPECT_FALSE(b->state().isPressed());
		EXPECT_TRUE(a->state().isPressed());

		mouse(150, 150, false, 3u);
		EXPECT_FALSE(a->state().isPressed());
	}

	TEST_F(RouterFixture, CaptureRoutesMoveToCapturer) {
		b->down = EventReply::handled().withCapture();
		b->move = EventReply::consumed();
		mouse(5, 5, true);
		EXPECT_EQ(router.captured(), b);

		log.clear();
		move(5.F, 5.F);
		EXPECT_EQ(filtered("move"), (Log { "move:b" }));
	}

	TEST_F(RouterFixture, CaptureReleasedOnButtonUpOutside) {
		b->down = EventReply::handled().withCapture();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		mouse(-5, -5, false);
		EXPECT_EQ(router.captured(), nullptr);
	}

	TEST_F(RouterFixture, CaptureTargetUnmounted) {
		b->down = EventReply::handled().withCapture();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		a->removeChild(b);
		log.clear();
		move(5.F, 5.F);
		EXPECT_EQ(router.captured(), nullptr);
		EXPECT_EQ(filtered("move"), (Log { "move:a" }));
	}

	TEST_F(RouterFixture, TopLayerHitFirst) {
		const auto popup = make_sptr<Leaf>("popup", &log);
		size(popup, 50.F);
		const auto host = make_sptr<Host>(popup, math::vec2 { 0.F }, math::vec2 { 50.F });
		[[maybe_unused]] const auto layer = window->requestLayer(LayerBand::ePopup, host);
		tick(window);

		mouse(5, 5, true);
		EXPECT_EQ(filtered("down"), (Log { "down:popup" }));
		window->dropLayer(host.get());
	}

	TEST_F(RouterFixture, KeyBubblesFromFocusTarget) {
		router.focus(window, c);
		router.route(window, engine::input::event::KeyboardEvent { 'a', true, 0u });
		EXPECT_EQ(log, (Log { "preview-key:a", "preview-key:b", "preview-key:c", "key-down:c", "key-down:b", "key-down:a" }));
	}

	TEST_F(RouterFixture, CaptureFollowsAcrossWindows) {
		auto other = make_sptr<engine::reflow::Window>();
		other->setClientSize(math::vec2 { 320.F, 240.F });
		other->setContent(make_sptr<Leaf>("o", &log));
		router.registerWindow(other);
		tick(other);

		b->down = EventReply::handled().withCapture();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		log.clear();
		router.route(other, engine::input::event::MouseButtonEvent { math::ivec2 { 5, 5 }, 0u, false, 0u });
		EXPECT_EQ(filtered("up"), (Log { "up:b", "up:a" }));
		EXPECT_EQ(router.captured(), nullptr);

		router.unregisterWindow(other.get());
	}

	TEST_F(RouterFixture, ReleaseOutsideEveryWindowOwnedByPress) {
		mouse(5, 5, true);
		ASSERT_EQ(router.pointerOwner(), window);

		router.route(router.pointerOwner(), engine::input::event::MouseButtonEvent { math::ivec2 { -5, -5 }, 0u, false, 0u });
		EXPECT_FALSE(c->state().isPressed());
		EXPECT_FALSE(a->state().isPressed());
		EXPECT_EQ(router.pointerOwner(), nullptr);
	}

	TEST_F(RouterFixture, PointerOwnerDropsUnmountedCapture) {
		b->down = EventReply::handled().withCapture();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		a->removeChild(b);
		EXPECT_EQ(router.pointerOwner(), window);
		EXPECT_EQ(router.captured(), nullptr);
	}
}
