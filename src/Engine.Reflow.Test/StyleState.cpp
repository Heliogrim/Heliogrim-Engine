#include "pch.h"

/**/

#include "RouterFixture.hpp"

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	namespace {
		class HookProbe final :
			public uikit::Paint {
		public:
			u32 calls = 0u;

			void onStyleStatesChanged() override {
				++calls;
			}
		};
	}

	namespace {
		class CountingPaint :
			public uikit::Paint {
		public:
			u32 calls = 0u;

			void onStyleStatesChanged() override {
				++calls;
			}
		};
	}

	TEST_F(RouterFixture, StyleStateRouterHookFiresOncePerRealChange) {
		auto leaf = make_sptr<ProbeWidget<CountingPaint>>("p", &log);
		size(leaf, 100.F);
		mount(leaf);

		move(5.F, 5.F);
		const auto afterEnter = leaf->calls;
		EXPECT_GE(afterEnter, 1u);
		move(6.F, 6.F);
		EXPECT_EQ(leaf->calls, afterEnter);

		mouse(5, 5, true, 0u);
		const auto afterPress = leaf->calls;
		EXPECT_GT(afterPress, afterEnter);
		mouse(5, 5, true, 1u);
		EXPECT_EQ(leaf->calls, afterPress);
	}

	TEST_F(RouterFixture, StyleStateRouterStatesAppear) {
		c->focusable = true;
		move(5.F, 5.F);
		mouse(5, 5, true);
		mouse(5, 5, false);

		const auto states = c->styleStates();
		EXPECT_TRUE(states & InteractiveStateFlagBits::eHover);
		EXPECT_TRUE(states & InteractiveStateFlagBits::eFocus);
		EXPECT_FALSE(states & InteractiveStateFlagBits::ePress);
	}

	TEST_F(RouterFixture, StyleStateStoredStatesCombine) {
		c->setStyleState(InteractiveStateFlagBits::eSelected, true);
		c->setStyleState(InteractiveStateFlagBits::eInvalid, true);

		const auto states = c->styleStates();
		EXPECT_TRUE(states & InteractiveStateFlagBits::eSelected);
		EXPECT_TRUE(states & InteractiveStateFlagBits::eInvalid);
		EXPECT_FALSE(states & InteractiveStateFlagBits::eDisable);
	}

	TEST_F(RouterFixture, StyleStateChangeHookFiresOncePerChange) {
		HookProbe probe {};
		probe.setStyleState(InteractiveStateFlagBits::eDisable, true);
		probe.setStyleState(InteractiveStateFlagBits::eDisable, true);
		EXPECT_EQ(probe.calls, 1u);

		probe.setStyleState(InteractiveStateFlagBits::eDisable, false);
		EXPECT_EQ(probe.calls, 2u);
	}
}
