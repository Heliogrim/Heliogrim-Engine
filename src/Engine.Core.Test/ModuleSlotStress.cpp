#include "pch.h"

/**/

import Engine.Common;
import Engine.Core;
import std;

using namespace hg;
using namespace hg::engine::core;

namespace Core {
	namespace {
		class StressRoot final :
			public RootModule {
			friend class ::hg::engine::core::ModuleSlotAccess;

		private:
			static constinit std::atomic<ptr<StressRoot>> _moduleSlot;

		public:
			/* Set in the constructor, not a default member initializer, so the write is a real store before publish. */
			StressRoot() noexcept {
				sentinel = 42;
			}

		public:
			volatile int sentinel;
		};

		constinit std::atomic<ptr<StressRoot>> StressRoot::_moduleSlot { nullptr };
	}

	TEST(ModuleSlotStress, ReadersSeeNoneOrConstructedModule) {
		std::atomic<u64> hits { 0uLL };
		std::atomic<u64> misses { 0uLL };
		std::atomic<u64> torn { 0uLL };
		// A reader that loaded the pointer just before retract may still touch it (stop ordering is a spec non-goal), so the
		//  modules die only after the readers joined.
		Vector<uptr<StressRoot>> retired {};

		{
			Vector<std::jthread> readers {};
			for (auto i = 0; i < 4; ++i) {
				readers.emplace_back(
					[&](std::stop_token stop_) {
						u64 localHits = 0uLL, localMisses = 0uLL, localTorn = 0uLL;
						while (not stop_.stop_requested()) {
							if (const auto module = engine::module<StressRoot>()) {
								++localHits;
								localTorn += (module->sentinel != 42) ? 1uLL : 0uLL;
							} else {
								++localMisses;
							}
						}
						hits.fetch_add(localHits, std::memory_order_relaxed);
						misses.fetch_add(localMisses, std::memory_order_relaxed);
						torn.fetch_add(localTorn, std::memory_order_relaxed);
					}
				);
			}

			Modules modules {};
			for (auto round = 0; round < 10'000; ++round) {
				auto module = make_uptr<StressRoot>();
				modules.addRootModule(*module);
				std::this_thread::yield();
				modules.removeRootModule(*module);
				retired.push_back(std::move(module));
			}
		}

		EXPECT_EQ(torn.load(), 0uLL);
		EXPECT_GT(hits.load(), 0uLL);
		EXPECT_GT(misses.load(), 0uLL);
	}
}
