#pragma once

#include <gtest/gtest.h>

/**/

#include <Engine.Pedantic/Clone/Clone.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;
#endif

namespace ReflowModule {
	using namespace hg;

	/**
	 * Paint leaf which counts the calls the engine issues against it.
	 */
	class ProbePaint final :
		public engine::reflow::uikit::Paint {
	public:
		mutable u32 prefetches = 0u;
		u32 computes = 0u;
		u32 applies = 0u;
		u32 ticks = 0u;
		bool ticking = false;
		bool remarkInApply = false;

	public:
		[[nodiscard]] bool shouldTick() const noexcept override {
			return ticking;
		}

		void tick() override {
			++ticks;
			Paint::tick();
		}

		engine::reflow::PrefetchSizing prefetchSizing(
			const engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowState> state_
		) const override {
			++prefetches;
			return Paint::prefetchSizing(axis_, state_);
		}

		void computeSizing(
			const engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowPassState> passState_
		) override {
			++computes;
			Paint::computeSizing(axis_, passState_);
		}

		void applyLayout(ref<engine::reflow::ReflowState> state_) override {
			++applies;
			Paint::applyLayout(state_);
			if (std::exchange(remarkInApply, false)) {
				markLayoutPending();
			}
		}
	};

	class TreeFixture :
		public testing::Test {
	public:
		engine::reflow::LayoutContext ctx { .localOffset = math::vec2 { 0.F }, .localSize = math::vec2 { 320.F, 240.F }, .localScale = 1.F };
		engine::reflow::ReflowState state;

	public:
		void tick(const sptr<engine::reflow::Widget> root_) {
			engine::reflow::ReflowEngine::tick(state, root_, clone(ctx));
		}

		void TearDown() override {
			state.reset();
		}
	};
}
