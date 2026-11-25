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

		[[nodiscard]] sptr<RouterFixture::Leaf> modalLeaf(string name_, const ptr<Vector<string>> log_) {
			auto leaf = make_sptr<RouterFixture::Leaf>(std::move(name_), log_);
			RouterFixture::size(leaf, 40.F);
			leaf->focusable = true;
			return leaf;
		}

		[[nodiscard]] sptr<Window> makeWindow(const sptr<Widget> content_) {
			auto window = make_sptr<Window>();
			window->setClientSize(math::vec2 { 100.F, 100.F });
			window->setContent(content_);
			return window;
		}
	}

	class EscapeLeaf final :
		public uikit::Paint {
	public:
		std::function<void()> close {};

	public:
		[[nodiscard]] bool isFocusable() const noexcept override {
			return true;
		}

		EventReply invokeOnKeyDown(ref<const KeyboardEvent>) override {
			if (const auto fn = std::exchange(close, {})) {
				fn();
			}
			return EventReply::handled();
		}
	};

	class ModalFixture :
		public RouterFixture {
	public:
		// Lays out the modal hosts and centres them, a 40x40 modal covers (140, 100) to (180, 140).
		void settle() {
			tick(window);
			move(1.F, 1.F);
			tick(window);
			log.clear();
		}
	};

	TEST_F(ModalFixture, BlocksContentBelow) {
		const auto m = modalLeaf("m", &log);
		const auto handle = router.openModal(window, m, []() {});
		settle();

		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, handle.id);
		EXPECT_EQ(nearestScope(m)->kind(), FocusScopeKind::eDialog);
		EXPECT_EQ(window->getFocusTarget(), m);

		mouse(5, 5, true);
		mouse(5, 5, false);
		EXPECT_TRUE(filtered("down:").empty());
		EXPECT_TRUE(filtered("preview-down:").empty());

		mouse(150, 110, true);
		mouse(150, 110, false);
		EXPECT_EQ(filtered("down:"), Vector<string> { "down:m" });

		router.closeModal(handle);
		EXPECT_FALSE(router.activeModal().has_value());
	}

	TEST_F(ModalFixture, BlocksOtherWindows) {
		const auto other = makeWindow(modalLeaf("other", &log));
		router.registerWindow(other);
		tick(other);

		const auto handle = router.openModal(window, modalLeaf("m", &log), []() {});
		settle();

		const auto reply = router.route(other, engine::input::event::MouseButtonEvent { math::ivec2 { 5, 5 }, 0u, true, 0u });
		router.route(other, engine::input::event::MouseButtonEvent { math::ivec2 { 5, 5 }, 0u, false, 0u });
		router.route(other, engine::input::event::MouseMoveEvent { math::fvec2 { 6.F, 6.F }, math::fvec2 { 0.F }, 0u, 0u });

		EXPECT_EQ(reply.response, EventResponse::eConsumed);
		EXPECT_TRUE(log.empty());
		EXPECT_EQ(router.activeModal()->id, handle.id);

		router.closeModal(handle);
		router.unregisterWindow(other.get());
	}

	TEST_F(ModalFixture, KeysGoToModalWindow) {
		const auto otherLeaf = modalLeaf("other", &log);
		const auto other = makeWindow(otherLeaf);
		router.registerWindow(other);
		tick(other);
		router.focus(other, otherLeaf);

		const auto handle = router.openModal(window, modalLeaf("m", &log), []() {});
		settle();

		router.route(other, engine::input::event::KeyboardEvent { 'x', true, 0u });

		EXPECT_EQ(filtered("key-down:"), Vector<string> { "key-down:m" });

		router.closeModal(handle);
		router.unregisterWindow(other.get());
	}

	TEST_F(ModalFixture, TabTrapped) {
		c->focusable = true;

		const auto box = make_sptr<Container>("box", &log);
		size(box, 80.F);
		const auto m1 = modalLeaf("m1", &log);
		const auto m2 = modalLeaf("m2", &log);
		box->addChild(m1);
		box->addChild(m2);

		const auto handle = router.openModal(window, box, []() {});
		settle();
		ASSERT_EQ(window->getFocusTarget(), m1);

		key('\t');
		EXPECT_EQ(window->getFocusTarget(), m2);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), m1);
		key('\t', 0x0001u);
		EXPECT_EQ(window->getFocusTarget(), m2);

		router.closeModal(handle);
	}

	TEST_F(ModalFixture, QueueFifo) {
		Vector<string> order {};
		const auto first = router.openModal(window, modalLeaf("m1", &log), [&order]() { order.emplace_back("m1"); });
		const auto second = router.openModal(window, modalLeaf("m2", &log), [&order]() { order.emplace_back("m2"); });

		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, first.id);
		EXPECT_EQ(window->layers().size(), 1uLL);

		router.closeModal(first);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, second.id);
		EXPECT_EQ(window->layers().size(), 1uLL);

		router.closeModal(second);
		EXPECT_FALSE(router.activeModal().has_value());
		EXPECT_EQ(order, (Vector<string> { "m1", "m2" }));
	}

	TEST_F(ModalFixture, NestedFromModalHandler) {
		const auto m = modalLeaf("m", &log);
		const auto inner = modalLeaf("inner", &log);
		const auto outer = router.openModal(window, m, []() {});
		settle();

		ModalHandle nested {};
		m->downHook = [&]() { nested = router.openModal(window, inner, []() {}); };
		mouse(150, 110, true);
		mouse(150, 110, false);

		ASSERT_NE(nested.id, 0uLL);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, nested.id);
		EXPECT_EQ(window->getFocusTarget(), inner);

		router.closeModal(nested);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, outer.id);
		EXPECT_EQ(window->getFocusTarget(), m);

		router.closeModal(outer);
	}

	TEST_F(ModalFixture, FocusRestoredOnClose) {
		c->focusable = true;
		mouse(5, 5, true);
		mouse(5, 5, false);
		ASSERT_EQ(window->getFocusTarget(), c);

		bool closed = false;
		const auto handle = router.openModal(window, modalLeaf("m", &log), [&]() { closed = window->getFocusTarget() == c; });
		router.closeModal(handle);

		EXPECT_TRUE(closed);
		EXPECT_EQ(window->getFocusTarget(), c);
	}

	TEST_F(ModalFixture, EscapeClosesTopPopupOnly) {
		const auto m = modalLeaf("m", &log);
		const auto handle = router.openModal(window, m, []() {});
		settle();

		bool popupClosed = false;
		const auto& layout = m->getLayoutState();
		discard = router.openPopup(
			window,
			modalLeaf("p", &log),
			PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
			m,
			None,
			[&popupClosed]() { popupClosed = true; }
		);
		tick(window);

		key(escape);
		EXPECT_TRUE(popupClosed);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, handle.id);
		EXPECT_EQ(window->getFocusTarget(), m);

		key(escape);
		EXPECT_TRUE(router.activeModal().has_value());

		router.closeModal(handle);
	}

	TEST_F(ModalFixture, ModalOwnerWindowDestroyed) {
		auto other = makeWindow(modalLeaf("other", &log));
		router.registerWindow(other);
		tick(other);

		bool firstClosed = false;
		const auto m2 = modalLeaf("m2", &log);
		discard = router.openModal(other, modalLeaf("m1", &log), [&firstClosed]() { firstClosed = true; });
		const auto second = router.openModal(window, m2, []() {});

		router.unregisterWindow(other.get());
		other.reset();

		move(1.F, 1.F);

		EXPECT_TRUE(firstClosed);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, second.id);
		EXPECT_EQ(window->getFocusTarget(), m2);

		router.closeModal(second);
		EXPECT_FALSE(router.activeModal().has_value());
		EXPECT_NE(window->getFocusTarget(), nullptr);
	}

	TEST_F(ModalFixture, NestedFromPopupInsideModal) {
		const auto m = modalLeaf("m", &log);
		const auto p = modalLeaf("p", &log);
		const auto outer = router.openModal(window, m, []() {});
		settle();

		const auto& layout = m->getLayoutState();
		const auto popup = router.openPopup(
			window,
			p,
			PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
			m,
			None,
			[]() {}
		);
		settle();

		ModalHandle nested {};
		p->downHook = [&]() { nested = router.openModal(window, modalLeaf("inner", &log), []() {}); };
		mouse(150, 150, true);
		mouse(150, 150, false);

		ASSERT_NE(nested.id, 0uLL);
		ASSERT_TRUE(router.activeModal().has_value());
		EXPECT_EQ(router.activeModal()->id, nested.id);

		router.closeModal(nested);
		router.closePopup(popup);
		router.closeModal(outer);
	}

	TEST_F(ModalFixture, PopupBelowModalDoesNotStealFocus) {
		const auto& layout = c->getLayoutState();
		const auto popup = router.openPopup(
			window,
			modalLeaf("p", &log),
			PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
			c,
			None,
			[]() {}
		);
		tick(window);

		const auto m = modalLeaf("m", &log);
		const auto handle = router.openModal(window, m, []() {});
		settle();
		ASSERT_EQ(window->getFocusTarget(), m);

		router.closePopup(popup);
		EXPECT_EQ(window->getFocusTarget(), m);

		router.closeModal(handle);
	}

	TEST_F(ModalFixture, ClickIntoModalNotConsumedByPopupBelow) {
		bool popupClosed = false;
		const auto& layout = c->getLayoutState();
		const auto popup = router.openPopup(
			window,
			modalLeaf("p", &log),
			PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
			c,
			None,
			[&popupClosed]() { popupClosed = true; }
		);
		tick(window);

		const auto handle = router.openModal(window, modalLeaf("m", &log), []() {});
		settle();

		mouse(150, 110, true);
		mouse(150, 110, false);
		EXPECT_EQ(filtered("down:"), Vector<string> { "down:m" });
		EXPECT_FALSE(popupClosed);

		key(escape);
		EXPECT_FALSE(popupClosed);

		router.closeModal(handle);
		router.closePopup(popup);
	}

	TEST_F(ModalFixture, NotificationBandStaysLive) {
		const auto handle = router.openModal(window, modalLeaf("m", &log), []() {});

		const auto n = modalLeaf("n", &log);
		const auto host = make_sptr<Host>(n, math::vec2 { 0.F }, window->getClientSize());
		discard = window->requestLayer(LayerBand::eNotification, host);
		n->setParent(host);
		host->markAsPending(true, true);
		settle();

		mouse(5, 5, true);
		mouse(5, 5, false);

		EXPECT_EQ(filtered("down:"), Vector<string> { "down:n" });

		window->dropLayer(host.get());
		router.closeModal(handle);
	}

	TEST_F(ModalFixture, OpeningReleasesCapture) {
		b->down = EventReply::handled().withCapture();
		mouse(5, 5, true);
		ASSERT_EQ(router.captured(), b);

		const auto handle = router.openModal(window, modalLeaf("m", &log), []() {});
		EXPECT_EQ(router.captured(), nullptr);
		router.closeModal(handle);
	}

	TEST_F(ModalFixture, EscapeClosingModalKeepsPopups) {
		const auto p = modalLeaf("p", &log);
		const auto& layout = c->getLayoutState();
		discard = router.openPopup(
			window,
			p,
			PopupAnchor { .offset = layout.layoutOffset, .size = layout.layoutSize, .side = PopupSide::eBelow, .align = PopupAlign::eStart },
			c,
			None,
			[]() {}
		);

		// The dialog closes itself on Escape and only reports handled.
		const auto m = make_sptr<EscapeLeaf>();
		size(m, 40.F);
		const auto handle = router.openModal(window, m, []() {});
		m->close = [this, handle]() { router.closeModal(handle); };
		settle();
		ASSERT_EQ(window->getFocusTarget(), m);

		key(0x1B);

		EXPECT_FALSE(router.activeModal().has_value());
		EXPECT_TRUE(router.hasOpenPopup(window.get()));
	}
}
