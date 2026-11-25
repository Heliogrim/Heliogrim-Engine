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
		constexpr u16 shift = 0x0001u;

		[[nodiscard]] sptr<RouterFixture::Leaf> focusableLeaf(string name_, const ptr<Vector<string>> log_) {
			auto leaf = make_sptr<RouterFixture::Leaf>(std::move(name_), log_);
			RouterFixture::size(leaf, 20.F);
			leaf->focusable = true;
			return leaf;
		}

		[[nodiscard]] sptr<FocusScope> panel(cref<sptr<Widget>> content_) {
			auto scope = make_sptr<FocusScope>(FocusScopeKind::ePanel);
			scope->setContent(content_);
			return scope;
		}
	}

	// Window::setContent detaches the previous content, so a scope around it is mounted before it takes the content.
	[[nodiscard]] static sptr<FocusScope> mountPanel(ref<RouterFixture> fixture_, cref<sptr<Widget>> content_) {
		auto scope = make_sptr<FocusScope>(FocusScopeKind::ePanel);
		fixture_.mount(scope);
		scope->setContent(content_);
		fixture_.tick(fixture_.window);
		return scope;
	}

	TEST_F(RouterFixture, ClickFocusesDeepestFocusable) {
		b->focusable = true;
		c->focusable = true;

		mouse(5, 5, true);

		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_TRUE(c->state().isFocus());
		EXPECT_TRUE(b->state().isFocus());
		EXPECT_TRUE(a->state().isFocus());

		// Clicking the focused widget again keeps everything as is.
		mouse(5, 5, false);
		mouse(5, 5, true);
		EXPECT_EQ(window->getFocusTarget(), c);
	}

	TEST_F(RouterFixture, FocusIgnoresForeignTargets) {
		c->focusable = true;
		router.focus(window, c);
		ASSERT_EQ(window->getFocusTarget(), c);

		// Unmounted.
		const auto loose = focusableLeaf("loose", &log);
		router.focus(window, loose);
		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_FALSE(loose->state().isFocus());

		// Mounted in another window.
		const auto other = make_sptr<Window>();
		other->setClientSize(math::vec2 { 100.F, 100.F });
		const auto foreign = focusableLeaf("foreign", &log);
		other->setContent(foreign);
		router.focus(window, foreign);
		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_TRUE(c->state().isFocus());
		EXPECT_FALSE(foreign->state().isFocus());
	}

	TEST_F(RouterFixture, ClickOnNonFocusableUsesRemembered) {
		c->focusable = true;
		const auto scope = mountPanel(*this, a);

		mouse(5, 5, true);
		mouse(5, 5, false);
		ASSERT_EQ(window->getFocusTarget(), c);
		EXPECT_EQ(scope->remembered(), c);

		router.blur(window);
		ASSERT_EQ(window->getFocusTarget(), window);
		EXPECT_FALSE(c->state().isFocus());

		// a itself is not focusable, so the scope falls back to the remembered descendant.
		mouse(150, 150, true);
		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_TRUE(c->state().isFocus());
	}

	TEST_F(RouterFixture, TabCyclesAndWraps) {
		const auto i1 = focusableLeaf("i1", &log);
		const auto i2 = focusableLeaf("i2", &log);
		const auto i3 = focusableLeaf("i3", &log);
		a->clearChildren();
		a->addChild(i1);
		a->addChild(i2);
		a->addChild(i3);
		::hg::discard = mountPanel(*this, a);

		router.focus(window, i1);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), i2);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), i3);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), i1);
		EXPECT_FALSE(i3->state().isFocus());
		EXPECT_TRUE(i1->state().isFocus());
	}

	TEST_F(RouterFixture, ShiftTabBackwards) {
		const auto i1 = focusableLeaf("i1", &log);
		const auto i2 = focusableLeaf("i2", &log);
		const auto i3 = focusableLeaf("i3", &log);
		a->clearChildren();
		a->addChild(i1);
		a->addChild(i2);
		a->addChild(i3);
		::hg::discard = mountPanel(*this, a);

		router.focus(window, i1);
		key('\t', shift);
		EXPECT_EQ(window->getFocusTarget(), i3);
		key('\t', shift);
		EXPECT_EQ(window->getFocusTarget(), i2);
	}

	TEST_F(RouterFixture, TabSkipsHiddenAndDisabled) {
		const auto i1 = focusableLeaf("i1", &log);
		const auto hidden = focusableLeaf("hidden", &log);
		const auto disabled = uikit::makeInputText();
		const auto i3 = focusableLeaf("i3", &log);
		hidden->state().unset(WidgetStateFlagBits::eVisible);
		disabled->disable();
		a->clearChildren();
		a->addChild(i1);
		a->addChild(hidden);
		a->addChild(disabled);
		a->addChild(i3);
		::hg::discard = mountPanel(*this, a);

		router.focus(window, i1);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), i3);
	}

	TEST_F(RouterFixture, TabStaysInsideScope) {
		const auto x1 = focusableLeaf("x1", &log);
		const auto x2 = focusableLeaf("x2", &log);
		const auto y1 = focusableLeaf("y1", &log);
		const auto left = make_sptr<Container>("left", &log);
		const auto right = make_sptr<Container>("right", &log);
		left->addChild(x1);
		left->addChild(x2);
		right->addChild(y1);
		a->clearChildren();
		a->addChild(panel(left));
		a->addChild(panel(right));
		// a is the content already, mount(a) would detach it (Window::setContent).
		tick(window);

		router.focus(window, x2);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), x1);
		router.focus(window, y1);
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), y1);
	}

	TEST_F(RouterFixture, TabWithoutFocusableKeepsFocus) {
		router.focus(window, c);
		key('\t');
		key('\t', shift);
		EXPECT_EQ(window->getFocusTarget(), c);
		EXPECT_TRUE(c->state().isFocus());
	}

	TEST_F(RouterFixture, TextFieldPassesTab) {
		const auto text = uikit::makeInputText();
		const auto next = focusableLeaf("next", &log);
		a->clearChildren();
		a->addChild(text);
		a->addChild(next);
		::hg::discard = mountPanel(*this, a);

		router.focus(window, text);
		ASSERT_TRUE(text->state().isFocus());
		EXPECT_FALSE(text->invokeOnKeyDown(engine::input::event::KeyboardEvent { '\t', true, 0u }).isConsumed());
		key('\t');
		EXPECT_EQ(window->getFocusTarget(), next);
		EXPECT_FALSE(text->state().isFocus());
	}

	TEST_F(RouterFixture, FocusRestoreTargetGone) {
		router.focus(window, c);
		const auto snapshot = router.snapshotFocus(window);
		router.blur(window);

		b->removeChild(c);
		router.restoreFocus(window, snapshot);
		EXPECT_EQ(window->getFocusTarget(), b);
		EXPECT_TRUE(b->state().isFocus());

		// Nothing below the window survives, the content takes the focus.
		const auto other = make_sptr<Leaf>("other", &log);
		const auto gone = router.snapshotFocus(window);
		mount(other);
		router.restoreFocus(window, gone);
		EXPECT_EQ(window->getFocusTarget(), other);
	}

	TEST_F(RouterFixture, ReentrantFocusFromBlur) {
		const auto d = make_sptr<Leaf>("d", &log);
		const auto e = make_sptr<Leaf>("e", &log);
		a->addChild(d);
		a->addChild(e);
		tick(window);

		router.focus(window, c);
		c->blurHook = [&] { router.focus(window, d); };
		router.focus(window, e);

		EXPECT_EQ(window->getFocusTarget(), d);
		const auto& path = window->getFocusPath();
		ASSERT_EQ(path.size(), 3uLL);
		EXPECT_EQ(path[0].lock(), window);
		EXPECT_EQ(path[1].lock(), a);
		EXPECT_EQ(path[2].lock(), d);
		EXPECT_TRUE(d->state().isFocus());
		EXPECT_TRUE(a->state().isFocus());
		EXPECT_FALSE(b->state().isFocus());
		EXPECT_FALSE(c->state().isFocus());
		EXPECT_FALSE(e->state().isFocus());
	}
}
