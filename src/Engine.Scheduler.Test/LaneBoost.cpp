#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Lane.hpp>
#include <Engine.Scheduler/LanePolicyConfig.hpp>
#include <Engine.Scheduler/Process/BoostCell.hpp>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "TestCompScheduler.hpp"
#include "TestSchedulerEngine.hpp"

using namespace hg::engine::scheduler;
using namespace hg;
using namespace std::chrono_literals;

namespace SchedulerModule {
	namespace {
		using clock = std::chrono::steady_clock;
		using task_ptr = non_owning_rptr<const task::TaskDelegate>;

		task_ptr makeTask(const Lane lane_) {
			return task::make_task([] {}, lane_);
		}

		task_ptr popAt(ref<Schedule> schedule_, const clock::time_point now_) {
			task_ptr out = nullptr;
			Lane lane {};
			return schedule_.pop(now_, out, lane) ? out : nullptr;
		}

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

		template <typename Pred_>
		bool pollFor(const std::chrono::milliseconds limit_, Pred_&& pred_) {
			const auto until = clock::now() + limit_;
			while (not pred_() && clock::now() < until) {
				std::this_thread::yield();
			}
			return pred_();
		}

		void spinFor(const clock::duration span_) {
			const auto until = clock::now() + span_;
			while (clock::now() < until) {}
		}

		// A cell whose producer is a fresh task queued in lane_.
		Arci<BoostCell> queuedProducer(ref<Schedule> schedule_, const Lane lane_, ref<task_ptr> task_) {
			auto cell = Arci<BoostCell>::create();
			task_ = makeTask(lane_);
			cell->task.store(task_);
			EXPECT_TRUE(schedule_.pushBoostable(task_, cell.get()));
			return cell;
		}
	}

	TEST(LaneBoost, PromoteMovesAQueuedTask) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		for (u32 n = 0; n < 3u; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		}
		auto cell = Arci<BoostCell>::create();
		const auto task = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.pushBoostable(task, cell.get()));

		EXPECT_TRUE(schedule.promote(task, Lane::eFrame, cell.get()));
		EXPECT_EQ(task->lane(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 3uLL);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 1uLL);
		EXPECT_EQ(popAt(schedule, clock::now()), task);
		schedule.leave(Lane::eFrame);
		EXPECT_EQ(schedule.queued(), 3uLL);
	}

	TEST(LaneBoost, PromoteOfAnAbsentTaskIsFalse) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		const auto absent = makeTask(Lane::eBackground);

		EXPECT_FALSE(schedule.promote(absent, Lane::eFrame, Arci<BoostCell>::create().get()));
		EXPECT_EQ(absent->lane(), Lane::eBackground);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 0uLL);
	}

	TEST(LaneBoost, PromoteNeverLowers) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		auto cell = Arci<BoostCell>::create();
		const auto task = makeTask(Lane::eInteractive);
		ASSERT_TRUE(schedule.pushBoostable(task, cell.get()));

		EXPECT_FALSE(schedule.promote(task, Lane::eBackground, cell.get()));
		EXPECT_FALSE(schedule.promote(task, Lane::eInteractive, cell.get()));
		EXPECT_EQ(task->lane(), Lane::eInteractive);
		EXPECT_EQ(schedule.queued(Lane::eInteractive), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 0uLL);
	}

	TEST(LaneBoost, PushBoostableUsesTheRaisedLane) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		auto cell = Arci<BoostCell>::create();
		cell->raise(Lane::eFrame);
		const auto task = makeTask(Lane::eBackground);

		ASSERT_TRUE(schedule.pushBoostable(task, cell.get()));
		EXPECT_EQ(task->lane(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 0uLL);

		const auto plain = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.pushBoostable(plain, nullptr));
		EXPECT_EQ(schedule.queued(Lane::eBackground), 1uLL);
	}

	TEST(LaneBoost, PromoteUnderAForeignCellLeavesTheTask) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		// Stands for a reused address: the boost of cellB finds a task queued under cellA.
		auto cellA = Arci<BoostCell>::create();
		auto cellB = Arci<BoostCell>::create();
		const auto task = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.pushBoostable(task, cellA.get()));

		EXPECT_FALSE(schedule.promote(task, Lane::eFrame, cellB.get()));
		EXPECT_EQ(task->lane(), Lane::eBackground);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 0uLL);

		EXPECT_TRUE(schedule.promote(task, Lane::eFrame, cellA.get()));
		EXPECT_EQ(task->lane(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 0uLL);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 1uLL);
	}

	TEST(LaneBoost, YieldedProducerIsPromoted) {
		constexpr u32 flood = 100u;
		std::atomic<u32> finished { 0u };
		std::atomic<u32> producerRank { ~0u };
		std::atomic<u32> firstFloodRank { ~0u };
		std::atomic_bool started { false };
		std::atomic_bool go { false };
		std::atomic_bool done { false };
		auto cell = Arci<BoostCell>::create();

		structuredRuntimeTest(
			1u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = boostScheduleOf(s_);

				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								fiber::self::bindBoost(cell.get());
								cell->task.store(fiber::self::current()->task);
								started = true;
								// no checkpoint until the flood and the waiter are queued
								while (not go.load()) {}
								// yields once, behind the flood, until the waiter's boost moves it to Frame
								while (fiber::self::effectiveLane() != Lane::eFrame) {
									fiber::self::checkpoint();
								}
								cell->task.store(nullptr);
								producerRank = finished.fetch_add(1u);
								done = true;
							},
							Lane::eBackground
						)
					)
				);
				ASSERT_TRUE(waitUntil(1000ms, started));

				for (u32 n = 0; n < flood; ++n) {
					ASSERT_TRUE(
						schedule->push(
							task::make_task(
								[&] {
									const auto rank = finished.fetch_add(1u);
									auto expected = ~0u;
									firstFloodRank.compare_exchange_strong(expected, rank);
								},
								Lane::eBackground
							)
						)
					);
				}
				ASSERT_TRUE(schedule->push(task::make_task([&] { boost(cell, Lane::eFrame); }, Lane::eFrame)));
				go = true;

				EXPECT_TRUE(waitUntil(2000ms, done));
				EXPECT_TRUE(pollFor(2000ms, [&] { return finished.load() == flood + 1u; }));
			}
		);

		EXPECT_EQ(producerRank.load(), 0u);
		EXPECT_EQ(firstFloodRank.load(), 1u);
	}

	TEST(LaneBoost, RunningProducerRequeuesInTheRaisedLane) {
		std::atomic_bool started { false };
		std::atomic_bool boosted { false };
		std::atomic_bool done { false };
		std::atomic<u32> yieldsAfterBoost { 0u };
		std::atomic<Lane> laneAfter { Lane::eInherit };
		auto cell = Arci<BoostCell>::create();

		structuredRuntimeTest(
			2u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = boostScheduleOf(s_);

				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								auto* const self = fiber::self::current();
								fiber::self::bindBoost(cell.get());
								cell->task.store(self->task);
								started = true;

								while (not boosted.load()) {
									spinFor(50us);
									fiber::self::checkpoint();
								}

								// the next checkpoint re-queues once in Frame; Frame never slices after that
								auto lastStart = self->sliceStart;
								const auto until = clock::now() + 5ms;
								while (clock::now() < until) {
									spinFor(50us);
									fiber::self::checkpoint();
									if (self->sliceStart != lastStart) {
										lastStart = self->sliceStart;
										++yieldsAfterBoost;
									}
								}
								laneAfter = self->task->lane();
								cell->task.store(nullptr);
								done = true;
							},
							Lane::eBackground
						)
					)
				);
				ASSERT_TRUE(waitUntil(1000ms, started));
				thread::self::sleepFor(2);

				// the producer is on a worker: the search finds nothing, its next re-push sees the raised lane
				boost(cell, Lane::eFrame, schedule);
				boosted = true;

				EXPECT_TRUE(waitUntil(2000ms, done));
			}
		);

		EXPECT_EQ(laneAfter.load(), Lane::eFrame);
		EXPECT_LE(yieldsAfterBoost.load(), 1u);
	}

	TEST(LaneBoost, BoostFollowsDependency) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		task_ptr producer = nullptr;
		auto b = queuedProducer(schedule, Lane::eBackground, producer);
		auto a = Arci<BoostCell>::create();
		a->setDependency(b);

		boost(a, Lane::eFrame, &schedule);

		EXPECT_EQ(a->lane.load(), Lane::eFrame);
		EXPECT_EQ(b->lane.load(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 0uLL);
		EXPECT_EQ(popAt(schedule, clock::now()), producer);
		schedule.leave(Lane::eFrame);
	}

	TEST(LaneBoost, SecondBoostOfARaisedCellDoesNothing) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		task_ptr producer = nullptr;
		auto cell = queuedProducer(schedule, Lane::eBackground, producer);

		boost(cell, Lane::eFrame, &schedule);
		ASSERT_EQ(popAt(schedule, clock::now()), producer);
		schedule.leave(Lane::eFrame);

		// re-queue the same pointer as a lower decoy: a second walk that searched would move it
		const_cast<ptr<task::TaskDelegate>>(producer)->lane() = Lane::eBackground;
		ASSERT_TRUE(schedule.push(producer));

		boost(cell, Lane::eFrame, &schedule);
		boost(cell, Lane::eInteractive, &schedule);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 0uLL);
	}

	TEST(LaneBoost, CycleOfDependenciesTerminates) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		auto a = Arci<BoostCell>::create();
		auto b = Arci<BoostCell>::create();
		a->setDependency(b);
		b->setDependency(a);

		boost(a, Lane::eFrame, &schedule);

		EXPECT_EQ(a->lane.load(), Lane::eFrame);
		EXPECT_EQ(b->lane.load(), Lane::eFrame);

		// break the cycle, or the cells keep each other alive
		a->setDependency(nullptr);
		b->setDependency(nullptr);
	}

	TEST(LaneBoost, BoostWalksAChainLongerThanSixteen) {
		constexpr u32 chain = 40u;
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		for (u32 n = 0; n < 3u; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		}
		task_ptr producer = nullptr;
		auto last = queuedProducer(schedule, Lane::eBackground, producer);

		std::vector<Arci<BoostCell>> cells {};
		for (u32 n = 0; n + 1u < chain; ++n) {
			cells.emplace_back(Arci<BoostCell>::create());
		}
		cells.emplace_back(last);
		for (u32 n = 0; n + 1u < chain; ++n) {
			cells[n]->setDependency(cells[n + 1u]);
		}

		boost(cells.front(), Lane::eFrame, &schedule);

		EXPECT_EQ(last->lane.load(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 3uLL);
		EXPECT_EQ(popAt(schedule, clock::now()), producer);
		schedule.leave(Lane::eFrame);

		for (auto& cell : cells) {
			cell->setDependency(nullptr);
		}
	}

	TEST(LaneBoost, LinkAfterBoostEscalatesTheDependency) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		auto a = Arci<BoostCell>::create();
		boost(a, Lane::eFrame, &schedule);

		task_ptr producer = nullptr;
		auto b = queuedProducer(schedule, Lane::eBackground, producer);
		linkDependency(*a, b, &schedule);

		EXPECT_EQ(b->lane.load(), Lane::eFrame);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 0uLL);
		EXPECT_EQ(popAt(schedule, clock::now()), producer);
		schedule.leave(Lane::eFrame);
	}

	TEST(LaneBoost, BoostedProducerChildrenInherit) {
		std::atomic<Lane> seen { Lane::eInherit };
		std::atomic_bool done { false };
		auto cell = Arci<BoostCell>::create();

		structuredRuntimeTest(
			2u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				ASSERT_TRUE(
					boostScheduleOf(s_)->push(
						task::make_task(
							[&, s = s_.get()] {
								fiber::self::bindBoost(cell.get());
								boost(cell, Lane::eFrame);
								s->exec(
									task::make_task(
										[&] {
											seen = fiber::self::current()->task->lane();
											done = true;
										}
									)
								);
							},
							Lane::eBackground
						)
					)
				);
				EXPECT_TRUE(waitUntil(2000ms, done));
			}
		);

		EXPECT_EQ(seen.load(), Lane::eFrame);
	}

	TEST(LaneBoost, DestroyDuringBoostReturns) {
		constexpr u32 count = 64u;
		std::vector<Arci<BoostCell>> cells {};
		for (u32 n = 0; n < count; ++n) {
			cells.emplace_back(Arci<BoostCell>::create());
		}
		clock::time_point stopAt {};

		structuredRuntimeTest(
			2u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = boostScheduleOf(s_);

				for (u32 n = 0; n < count; ++n) {
					auto* const cell = cells[n].get();
					const auto producer = task::make_task(
						[cell] {
							cell->task.store(nullptr);
							spinFor(100us);
						},
						Lane::eBackground
					);
					cell->task.store(producer);
					ASSERT_TRUE(schedule->push(producer));
					ASSERT_TRUE(schedule->push(task::make_task([] { spinFor(100us); }, Lane::eInteractive)));
					// the waiters boost from workers, racing the stop below
					ASSERT_TRUE(schedule->push(task::make_task([&cells, n] { boost(cells[n], Lane::eFrame); }, Lane::eFrame)));
				}
				stopAt = clock::now();
			}
		);

		EXPECT_LE(clock::now() - stopAt, 2s);
	}

	TEST(LaneBoost, CellOwnedOnlyByTheTaskSurvivesRepush) {
		constexpr u32 rounds = 3000u;
		constexpr u32 yields = 8u;
		std::atomic<u32> finished { 0u };

		structuredRuntimeTest(
			4u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = boostScheduleOf(s_);

				for (u32 n = 0; n < rounds; ++n) {
					auto cell = Arci<BoostCell>::create();
					auto boosted = std::make_shared<std::atomic_bool>(false);
					// the closure is the cell's only owner once this loop drops its reference; the waiter borrows it and
					// the producer outlives the borrow, so a repush after the boost runs with no other owner
					const auto producer = task::make_task(
						[cell, boosted, schedule, &finished] {
							fiber::self::bindBoost(cell.get());
							cell->task.store(fiber::self::current()->task);
							(void)schedule->push(
								task::make_task(
									[waiter = cell.get(), boosted] {
										boost(waiter->arci_from_this(), Lane::eFrame);
										boosted->store(true);
									},
									Lane::eFrame
								)
							);
							for (u32 y = 0; y < yields || not boosted->load(); ++y) {
								fiber::self::yield();
							}
							cell->task.store(nullptr);
							finished.fetch_add(1u);
						},
						Lane::eBackground
					);
					cell.reset();
					ASSERT_TRUE(schedule->push(producer));
				}

				EXPECT_TRUE(pollFor(20000ms, [&] { return finished.load() == rounds; }));
			}
		);

		EXPECT_EQ(finished.load(), rounds);
	}

	TEST(LaneBoost, TryRaiseOnlyRaises) {
		auto cell = Arci<BoostCell>::create();
		ASSERT_EQ(cell->lane.load(), Lane::eBackground);

		EXPECT_TRUE(cell->tryRaise(Lane::eInteractive));
		EXPECT_EQ(cell->lane.load(), Lane::eInteractive);

		EXPECT_FALSE(cell->tryRaise(Lane::eBackground));
		EXPECT_FALSE(cell->tryRaise(Lane::eInteractive));
		EXPECT_EQ(cell->lane.load(), Lane::eInteractive);

		EXPECT_TRUE(cell->tryRaise(Lane::eFrame));
		EXPECT_EQ(cell->lane.load(), Lane::eFrame);
		EXPECT_FALSE(cell->tryRaise(Lane::eFrame));
		EXPECT_EQ(cell->lane.load(), Lane::eFrame);
	}
}
