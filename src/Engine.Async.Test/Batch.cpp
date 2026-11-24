#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

import Engine.Scheduler;
import Heliogrim;

using namespace hg;

namespace AsyncModule {
	namespace {
		template <typename Pred_>
		bool waitUntil(Pred_&& pred_) {
			const auto until = std::chrono::steady_clock::now() + std::chrono::seconds { 5 };
			while (not pred_() && std::chrono::steady_clock::now() < until) {
				engine::scheduler::thread::self::sleepFor(1);
			}
			return pred_();
		}
	}

	TEST(AsyncBatch, AwaitLater) {
		struct State {
			std::vector<std::atomic<u32>> hits = std::vector<std::atomic<u32>>(32uL);
			std::atomic_bool gate { false };
			std::atomic_bool entered { false };
			std::atomic_bool returned { false };
		};
		const auto state = std::make_shared<State>();

		SchedulerModule::structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					engine::scheduler::task::make_task(
						[state] {
							// Indices block on the gate, so await must park before the batch can complete.
							auto b = hg::batch(
								BatchCount { 32u },
								[state](const u16 index_, u16) {
									const auto until = std::chrono::steady_clock::now() + std::chrono::seconds { 5 };
									while (not state->gate.load() && std::chrono::steady_clock::now() < until) {
										engine::scheduler::thread::self::sleepFor(1);
									}
									state->hits[index_].fetch_add(1uL);
								}
							);
							state->entered.store(true);
							hg::await(b);
							state->returned.store(true);
						}
					)
				);

				ASSERT_TRUE(waitUntil([&] { return state->entered.load(); }));
				engine::scheduler::thread::self::sleepFor(20);
				EXPECT_FALSE(state->returned.load());

				state->gate.store(true);
				EXPECT_TRUE(waitUntil([&] { return state->returned.load(); }));
			}
		);
		for (const auto& hit : state->hits) {
			EXPECT_EQ(hit.load(), 1uL);
		}
	}

	TEST(AsyncBatch, NestedParallelForOneWorker) {
		struct State {
			std::atomic<u32> hits { 0uL };
			std::atomic_bool done { false };
		};
		const auto state = std::make_shared<State>();

		SchedulerModule::structuredRuntimeTest(
			1u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					engine::scheduler::task::make_task(
						[state] {
							hg::parallelFor(
								BatchCount { 4u },
								[state](u16, u16) {
									hg::parallelFor(BatchCount { 4u }, [state](u16, u16) { state->hits.fetch_add(1uL); });
								}
							);
							state->done.store(true);
						}
					)
				);
				EXPECT_TRUE(waitUntil([&] { return state->done.load(); }));
			}
		);
		EXPECT_EQ(state->hits.load(), 16uL);
	}
}
