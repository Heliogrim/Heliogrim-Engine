#include "pch.h"

/**/

#include "../Engine.Scheduler.Test/TestCompScheduler.hpp"
#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

import Engine.Async;
import Engine.Scheduler;
import std;

using namespace hg::engine::scheduler;
using namespace hg;
using namespace std::chrono_literals;

namespace AsyncModule {
	namespace {
		using clock = std::chrono::steady_clock;

		constexpr u32 floodCount = 1'000u;

		ptr<Schedule> boostScheduleOf(nmpt<engine::Scheduler> s_) {
			return static_cast<ptr<engine::TestCompScheduler>>(s_.get())->testGetSchedule();
		}

		bool waitUntil(const std::chrono::milliseconds limit_, const std::atomic_bool& flag_) {
			const auto until = clock::now() + limit_;
			while (not flag_.load() && clock::now() < until) {
				std::this_thread::yield();
			}
			return flag_.load();
		}

		void spinFor(const clock::duration span_) {
			const auto until = clock::now() + span_;
			while (clock::now() < until) {}
		}

		void pushIn(ref<Schedule> schedule_, const Lane lane_, std::function<void()>&& fn_) {
			EXPECT_TRUE(schedule_.push(task::make_task(std::move(fn_), lane_)));
		}

		// Holds the only worker until the returned gate opens.
		void holdWorker(ref<Schedule> schedule_, ref<std::atomic_bool> held_, ref<const std::atomic_bool> gate_) {
			pushIn(
				schedule_,
				Lane::eFrame,
				[&held_, &gate_] {
					held_.store(true);
					while (not gate_.load()) {
						std::this_thread::yield();
					}
				}
			);
			EXPECT_TRUE(waitUntil(5'000ms, held_));
		}

		void flood(ref<Schedule> schedule_, const clock::duration span_) {
			for (u32 n = 0; n < floodCount; ++n) {
				pushIn(schedule_, Lane::eBackground, [span_] { spinFor(span_); });
			}
		}

		// Each level flattens the next one, so completing the innermost forwards through every level.
		Future<int> nestedChain(const int depth_) {
			if (depth_ == 0) {
				return Future<int>::ready(42);
			}
			return Future<void>::ready().then([depth_] { return nestedChain(depth_ - 1); });
		}
	}

	TEST(FutureBoost, QueuedProducerIsBoostedByAFrameWaiter) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic_bool held { false };
				std::atomic_bool gate { false };
				holdWorker(schedule, held, gate);

				flood(schedule, 50us);
				auto produced = std::make_shared<Future<int>>(async([] { return 7; }, Lane::eBackground));

				std::atomic_bool returned { false };
				std::atomic<u64> queuedAtReturn { 0uLL };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&, produced] {
						auto result = await(std::move(*produced));
						queuedAtReturn.store(schedule.queued(Lane::eBackground));
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);
				gate.store(true);

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 7);
				EXPECT_GE(queuedAtReturn.load(), 900uLL);
				while (schedule.queued() != 0uLL) {
					std::this_thread::yield();
				}
				thread::self::sleepFor(20);
			}
		);
	}

	TEST(FutureBoost, ChainIsScheduledInTheWaitersLane) {
		SchedulerModule::structuredRuntimeTest(
			2u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				Promise<int> promise {};
				std::atomic<Lane> laneB { Lane::eInherit };
				std::atomic<Lane> laneC { Lane::eInherit };
				std::shared_ptr<Future<int>> chain {};
				std::atomic_bool built { false };

				pushIn(
					schedule,
					Lane::eBackground,
					[&] {
						chain = std::make_shared<Future<int>>(
							promise.future().then(
								[&](int v_) {
									laneB.store(fiber::self::effectiveLane());
									return v_ + 1;
								}
							).then(
								[&](int v_) {
									laneC.store(fiber::self::effectiveLane());
									return v_ + 1;
								}
							)
						);
						built.store(true);
					}
				);
				ASSERT_TRUE(waitUntil(5'000ms, built));

				std::atomic_bool awaiting { false };
				std::atomic_bool returned { false };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&, chain] {
						awaiting.store(true);
						auto result = await(std::move(*chain));
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);
				ASSERT_TRUE(waitUntil(5'000ms, awaiting));
				thread::self::sleepFor(50);
				promise.resolve(1);

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 3);
				EXPECT_EQ(laneB.load(), Lane::eFrame);
				EXPECT_EQ(laneC.load(), Lane::eFrame);
			}
		);
	}

	TEST(FutureBoost, NestedWaitIsBoosted) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic_bool held { false };
				std::atomic_bool gate { false };
				holdWorker(schedule, held, gate);

				// queued ahead of the flood; its own Background future is created behind it
				auto produced = std::make_shared<Future<int>>(
					async(
						[] {
							auto inner = await(async([] { return 5; }, Lane::eBackground));
							return inner.has_value() ? inner.value() + 1 : -1;
						},
						Lane::eBackground
					)
				);
				flood(schedule, 50us);

				std::atomic_bool returned { false };
				std::atomic<u64> queuedAtReturn { 0uLL };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&, produced] {
						auto result = await(std::move(*produced));
						queuedAtReturn.store(schedule.queued(Lane::eBackground));
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);
				gate.store(true);

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 6);
				EXPECT_GE(queuedAtReturn.load(), 900uLL);
				while (schedule.queued() != 0uLL) {
					std::this_thread::yield();
				}
				thread::self::sleepFor(20);
			}
		);
	}

	TEST(FutureBoost, FlattenedPromiseFutureHasNoLowerLaneHop) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic_bool held { false };
				std::atomic_bool gate { false };
				holdWorker(schedule, held, gate);

				Promise<int> promise {};
				std::atomic_bool forwarded { false };
				auto produced = std::make_shared<Future<int>>(
					async(
						[&] {
							forwarded.store(true);
							return promise.future();
						},
						Lane::eBackground
					)
				);
				flood(schedule, 200us);

				std::atomic_bool returned { false };
				std::atomic<u64> queuedAtReturn { 0uLL };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&, produced] {
						auto result = await(std::move(*produced));
						queuedAtReturn.store(schedule.queued(Lane::eBackground));
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);
				gate.store(true);

				ASSERT_TRUE(waitUntil(10'000ms, forwarded));
				std::thread { [&promise] { promise.resolve(11); } }.join();

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 11);
				EXPECT_GE(queuedAtReturn.load(), 900uLL);
				while (schedule.queued() != 0uLL) {
					std::this_thread::yield();
				}
				thread::self::sleepFor(20);
			}
		);
	}

	TEST(FutureBoost, NoProducerIsNoOp) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic_bool held { false };
				std::atomic_bool gate { false };
				holdWorker(schedule, held, gate);

				// from a plain thread: waits, never boosts; the queued producer must stay Background
				std::atomic<Lane> plainLane { Lane::eInherit };
				std::atomic<int> plainValue { 0 };
				std::thread plainWaiter {
					[&] {
						auto plain = await(
							async(
								[&] {
									plainLane.store(fiber::self::effectiveLane());
									return 3;
								},
								Lane::eBackground
							)
						);
						plainValue.store(plain.has_value() ? plain.value() : -1);
					}
				};
				thread::self::sleepFor(20);
				EXPECT_EQ(schedule.queued(Lane::eFrame), 0uLL);
				EXPECT_EQ(schedule.queued(Lane::eBackground), 1uLL);
				gate.store(true);
				plainWaiter.join();
				EXPECT_EQ(plainValue.load(), 3);
				EXPECT_EQ(plainLane.load(), Lane::eBackground);

				Promise<int> promise {};
				auto pending = std::make_shared<Future<int>>(promise.future());
				std::atomic_bool readyDone { false };
				std::atomic_bool awaiting { false };
				std::atomic<int> sum { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&, pending] {
						auto first = await(Future<int>::ready(20));
						sum.fetch_add(first.has_value() ? first.value() : -100);
						readyDone.store(true);
						awaiting.store(true);
						auto second = await(std::move(*pending));
						sum.fetch_add(second.has_value() ? second.value() : -100);
					}
				);
				ASSERT_TRUE(waitUntil(5'000ms, readyDone));
				ASSERT_TRUE(waitUntil(5'000ms, awaiting));
				thread::self::sleepFor(10);
				promise.resolve(22);

				const auto until = clock::now() + 5'000ms;
				while (sum.load() != 42 && clock::now() < until) {
					std::this_thread::yield();
				}
				EXPECT_EQ(sum.load(), 42);
			}
		);
	}

	TEST(FutureBoost, DeepFlattenChainDoesNotRecurse) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				constexpr int depth = 100'000;
				std::atomic_bool returned { false };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&] {
						auto result = await(nestedChain(depth));
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);

				ASSERT_TRUE(waitUntil(60'000ms, returned));
				EXPECT_EQ(value.load(), 42);
			}
		);
	}

	TEST(FutureBoost, FlattenWithoutWaiterDoesNotRaiseTheInnerProducer) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic<Lane> observed { Lane::eInherit };
				std::atomic_bool ran { false };
				std::shared_ptr<Future<int>> kept {};
				pushIn(
					schedule,
					Lane::eFrame,
					[&] {
						// built and kept, never awaited
						kept = std::make_shared<Future<int>>(
							Future<void>::ready().then(
								[&] {
									return async(
										[&] {
											observed.store(fiber::self::effectiveLane());
											ran.store(true);
											return 1;
										},
										Lane::eBackground
									);
								}
							)
						);
					}
				);

				ASSERT_TRUE(waitUntil(10'000ms, ran));
				EXPECT_EQ(observed.load(), Lane::eBackground);
				while (schedule.queued() != 0uLL) {
					std::this_thread::yield();
				}
				thread::self::sleepFor(20);
			}
		);
	}

	TEST(FutureBoost, FlattenUnderAFrameWaiterRaisesTheInnerProducer) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic<Lane> observed { Lane::eInherit };
				std::atomic_bool returned { false };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&] {
						// a Frame chain over a Background dependency, awaited from Frame
						auto result = await(
							Future<void>::ready().then(
								[&] {
									return async(
										[&] {
											observed.store(fiber::self::effectiveLane());
											return 1;
										},
										Lane::eBackground
									);
								}
							)
						);
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 1);
				EXPECT_EQ(observed.load(), Lane::eFrame);
			}
		);
	}

	TEST(FutureBoost, FlattenUnderAFrameWaiterRaisesTheInnerProducerOfABackgroundOuter) {
		SchedulerModule::structuredRuntimeTest(
			1u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				std::atomic<Lane> observed { Lane::eInherit };
				std::atomic_bool returned { false };
				std::atomic<int> value { 0 };
				pushIn(
					schedule,
					Lane::eFrame,
					[&] {
						// a Background outer, so the Frame waiter is what lifts it above its base lane
						auto result = await(
							Future<void>::ready(Lane::eBackground).then(
								[&] {
									return async(
										[&] {
											observed.store(fiber::self::effectiveLane());
											return 1;
										},
										Lane::eBackground
									);
								}
							)
						);
						value.store(result.has_value() ? result.value() : -1);
						returned.store(true);
					}
				);

				ASSERT_TRUE(waitUntil(10'000ms, returned));
				EXPECT_EQ(value.load(), 1);
				EXPECT_EQ(observed.load(), Lane::eFrame);
			}
		);
	}

	TEST(FutureBoost, ContinuationRunsExactlyOnceUnderRacingBoosts) {
		SchedulerModule::structuredRuntimeTest(
			8u,
			[](nmpt<engine::Scheduler> scheduler_) {
				thread::self::sleepFor(20);
				auto& schedule = *boostScheduleOf(scheduler_);

				constexpr u32 rounds = 10'000u;
				std::atomic<u32> counter { 0u };
				std::atomic<u32> waited { 0u };

				for (u32 round = 0; round < rounds; ++round) {
					Promise<int> promise {};
					auto chained = std::make_shared<Future<int>>(
						promise.future().then(
							[&counter](int v_) {
								counter.fetch_add(1u);
								return v_;
							}
						)
					);
					pushIn(
						schedule,
						Lane::eFrame,
						[&waited, chained] {
							(void)await(std::move(*chained));
							waited.fetch_add(1u);
						}
					);
					promise.resolve(static_cast<int>(round));
				}

				const auto until = clock::now() + 30'000ms;
				while (waited.load() != rounds && clock::now() < until) {
					std::this_thread::yield();
				}
				thread::self::sleepFor(20);
				EXPECT_EQ(waited.load(), rounds);
				EXPECT_EQ(counter.load(), rounds);
			}
		);
	}
}
