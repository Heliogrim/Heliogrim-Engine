#include "pch.h"

/**/

#include <Engine.Scheduler/Lane.hpp>
#include <Engine.Scheduler/LanePolicyConfig.hpp>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "TestCompScheduler.hpp"
#include "TestSchedulerEngine.hpp"

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <processthreadsapi.h>
#endif

import std;

using namespace hg::engine::scheduler;
using namespace hg;
using namespace std::chrono_literals;

namespace hg::engine::scheduler {
	// Test hook: counts a task into a lane's depth without inserting it, as a push between its count and its insert.
	struct ScheduleTestAccess {
		static void countWithoutInsert(ref<Schedule> schedule_, const Lane lane_) {
			schedule_._depth[static_cast<u64>(lane_)].fetch_add(1uLL, std::memory_order::acq_rel);
		}

		// Sets the last start of a lower lane (0 = Interactive, 1 = Background).
		static void setLastStart(ref<Schedule> schedule_, const u64 lowerIdx_, const Schedule::time_point at_) {
			schedule_._lastStart[lowerIdx_].store(at_.time_since_epoch().count(), std::memory_order::relaxed);
		}

		// The floor reference setup stamps for both lower lanes.
		static Schedule::time_point setupStamp(ref<const Schedule> schedule_) {
			return Schedule::time_point { Schedule::time_point::duration { schedule_._lastStart[0].load(std::memory_order::relaxed) } };
		}
	};
}

namespace SchedulerModule {
	namespace {
		using clock = std::chrono::steady_clock;
		using task_ptr = non_owning_rptr<const task::TaskDelegate>;

		task_ptr makeTask(const Lane lane_) {
			return task::make_task([] {}, lane_);
		}

		LanePolicyConfig fixedReserve(const u32 reserve_) {
			LanePolicyConfig config {};
			config.reserve = reserve_;
			return config;
		}

		task_ptr popAt(ref<Schedule> schedule_, const clock::time_point now_) {
			task_ptr out = nullptr;
			Lane lane {};
			return schedule_.pop(now_, out, lane) ? out : nullptr;
		}

		void runFrame(ref<Schedule> schedule_, const u32 width_) {
			for (u32 n = 0; n < width_; ++n) {
				EXPECT_TRUE(schedule_.push(makeTask(Lane::eFrame)));
			}
			for (u32 n = 0; n < width_; ++n) {
				EXPECT_NE(popAt(schedule_, clock::now()), nullptr);
				schedule_.leave(Lane::eFrame);
			}
			schedule_.rollFrame();
		}

		ptr<Schedule> scheduleOf(nmpt<engine::Scheduler> s_) {
			return static_cast<ptr<engine::TestCompScheduler>>(s_.get())->testGetSchedule();
		}

		bool pollFor(const std::chrono::milliseconds limit_, std::function<bool()>&& pred_) {
			const auto until = clock::now() + limit_;
			while (not pred_()) {
				if (clock::now() > until) {
					return pred_();
				}
				std::this_thread::yield();
			}
			return true;
		}

		void spinFor(const clock::duration span_) {
			const auto until = clock::now() + span_;
			while (clock::now() < until) {}
		}

		#ifdef WIN32
		std::chrono::milliseconds processCpuTime() {
			FILETIME creation {}, exit {}, kernel {}, user {};
			::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel, &user);
			const auto ticks = [](ref<const FILETIME> ft_) { return (static_cast<u64>(ft_.dwHighDateTime) << 32uLL) | ft_.dwLowDateTime; };
			// FILETIME counts 100 ns
			return std::chrono::milliseconds { (ticks(kernel) + ticks(user)) / 10'000uLL };
		}
		#else
		std::chrono::milliseconds processCpuTime() {
			return {};
		}
		#endif
	}

	TEST(LanePolicy, FloorServesBackgroundOncePerPeriodUnderFrameLoad) {
		Schedule schedule;
		schedule.setup(4u, LanePolicyConfig {});

		const auto b = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b));
		for (auto n = 0; n < 3; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		}

		const auto t0 = clock::now();
		const auto first = popAt(schedule, t0);
		ASSERT_NE(first, nullptr);
		EXPECT_EQ(first->lane(), Lane::eFrame);
		EXPECT_EQ(popAt(schedule, t0 + 100ms), b);
		const auto third = popAt(schedule, t0 + 100ms);
		ASSERT_NE(third, nullptr);
		EXPECT_EQ(third->lane(), Lane::eFrame);
	}

	TEST(LanePolicy, FloorTurnIsExclusiveForEqualNow) {
		for (auto round = 0; round < 50; ++round) {
			Schedule schedule;
			schedule.setup(4u, LanePolicyConfig {});

			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
			ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
			ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));

			const auto now = clock::now() + 100ms;
			std::atomic<int> gate { 0 };
			std::atomic<task_ptr> got[2] {};
			auto racer = [&](const int idx_) {
				gate.fetch_add(1);
				while (gate.load() < 2) {}
				got[idx_].store(popAt(schedule, now));
			};
			std::thread a { racer, 0 };
			std::thread c { racer, 1 };
			a.join();
			c.join();

			ASSERT_NE(got[0].load(), nullptr);
			ASSERT_NE(got[1].load(), nullptr);
			const auto backgrounds = (got[0].load()->lane() == Lane::eBackground ? 1 : 0)
				+ (got[1].load()->lane() == Lane::eBackground ? 1 : 0);
			EXPECT_EQ(backgrounds, 1);
		}
	}

	TEST(LanePolicy, StaleNowGetsNoFloorTurn) {
		Schedule schedule;
		schedule.setup(4u, LanePolicyConfig {});

		const auto b = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));

		const auto t1 = clock::now() + 100ms;
		EXPECT_EQ(popAt(schedule, t1), b);
		const auto stale = popAt(schedule, t1 - 1ms);
		ASSERT_NE(stale, nullptr);
		EXPECT_EQ(stale->lane(), Lane::eFrame);
	}

	TEST(LanePolicy, InteractiveFloorIsCheckedBeforeBackground) {
		Schedule schedule;
		schedule.setup(4u, LanePolicyConfig {});

		const auto b = makeTask(Lane::eBackground);
		const auto i = makeTask(Lane::eInteractive);
		ASSERT_TRUE(schedule.push(b));
		ASSERT_TRUE(schedule.push(i));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));

		const auto now = clock::now() + 100ms;
		EXPECT_EQ(popAt(schedule, now), i);
		EXPECT_EQ(popAt(schedule, now), b);
	}

	TEST(LanePolicy, FloorTurnOnEmptiedLaneFallsThrough) {
		Schedule schedule;
		schedule.setup(4u, LanePolicyConfig {});

		ScheduleTestAccess::countWithoutInsert(schedule, Lane::eBackground);
		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(f));

		EXPECT_EQ(popAt(schedule, clock::now() + 100ms), f);
	}

	TEST(LanePolicy, ReserveKeepsWorkersOffLowerLanes) {
		Schedule schedule;
		schedule.setup(3u, fixedReserve(2u));
		EXPECT_EQ(schedule.reserve(), 2u);

		const auto b0 = makeTask(Lane::eBackground);
		const auto b1 = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b0));
		ASSERT_TRUE(schedule.push(b1));

		const auto now = clock::now();
		EXPECT_EQ(popAt(schedule, now), b0);
		EXPECT_EQ(popAt(schedule, now), nullptr);
		schedule.leave(Lane::eBackground);
		EXPECT_EQ(popAt(schedule, now), b1);
	}

	TEST(LanePolicy, FixedReserveIsClampedToWorkersMinusOne) {
		{
			Schedule schedule;
			schedule.setup(1u, fixedReserve(2u));
			EXPECT_EQ(schedule.reserve(), 0u);
			const auto b = makeTask(Lane::eBackground);
			ASSERT_TRUE(schedule.push(b));
			EXPECT_EQ(popAt(schedule, clock::now()), b);
		}
		{
			Schedule schedule;
			schedule.setup(3u, fixedReserve(9u));
			EXPECT_EQ(schedule.reserve(), 2u);
		}
	}

	TEST(LanePolicy, ReserveIsOneBeforeTheFirstRoll) {
		Schedule schedule;
		schedule.setup(20u, LanePolicyConfig {});
		EXPECT_EQ(schedule.reserve(), 1u);
	}

	TEST(LanePolicy, ZeroFloorsAndSliceAreClampedToOne) {
		Schedule schedule;
		LanePolicyConfig config {};
		config.sliceMicros = 0uL;
		config.interactiveFloorMicros = 0uL;
		config.backgroundFloorMicros = 0uL;
		config.frameWaitLimitMicros = 0uL;
		schedule.setup(4u, config);
		EXPECT_EQ(schedule.sliceMicros(), 1u);

		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		ASSERT_TRUE(schedule.push(f));

		// At the setup stamp no time has passed: a floor of 0 is due (0 >= 0) and would hand out Interactive, a floor of 1 us is not.
		EXPECT_EQ(popAt(schedule, ScheduleTestAccess::setupStamp(schedule)), f);
	}

	TEST(LanePolicy, ReserveFollowsPeakFrameDemand) {
		{
			Schedule schedule;
			schedule.setup(20u, LanePolicyConfig {});
			runFrame(schedule, 8u);
			EXPECT_EQ(schedule.reserve(), 8u);
		}
		{
			Schedule schedule;
			schedule.setup(3u, LanePolicyConfig {});
			runFrame(schedule, 8u);
			EXPECT_EQ(schedule.reserve(), 2u);
		}
	}

	TEST(LanePolicy, ParkedParentDoesNotCountAsDemand) {
		Schedule schedule;
		schedule.setup(20u, LanePolicyConfig {});

		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		ASSERT_NE(popAt(schedule, clock::now()), nullptr);
		schedule.leave(Lane::eFrame);
		runFrame(schedule, 8u);
		EXPECT_EQ(schedule.reserve(), 8u);
	}

	TEST(LanePolicy, ChainGivesReserveOne) {
		Schedule schedule;
		schedule.setup(20u, LanePolicyConfig {});
		for (auto n = 0; n < 3; ++n) {
			runFrame(schedule, 1u);
		}
		EXPECT_EQ(schedule.reserve(), 1u);
	}

	TEST(LanePolicy, FanOutIsForgottenAfterTheWindow) {
		Schedule schedule;
		schedule.setup(20u, LanePolicyConfig {});
		runFrame(schedule, 8u);
		for (auto n = 0; n < 7; ++n) {
			runFrame(schedule, 1u);
		}
		EXPECT_EQ(schedule.reserve(), 8u);
		runFrame(schedule, 1u);
		EXPECT_EQ(schedule.reserve(), 1u);
	}

	TEST(LanePolicy, StaleReserveFallsBackToTheMinimum) {
		Schedule schedule;
		LanePolicyConfig config {};
		// keeps floor turns out of the pops below, so only the cap decides
		config.backgroundFloorMicros = 10'000'000uL;
		schedule.setup(8u, config);
		runFrame(schedule, 6u);
		const auto rolled = clock::now();
		ASSERT_EQ(schedule.reserve(), 6u);

		for (auto n = 0; n < 8; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		}

		EXPECT_EQ(schedule.effectiveReserve(rolled + 100ms), 6u);
		EXPECT_NE(popAt(schedule, rolled + 100ms), nullptr);
		EXPECT_NE(popAt(schedule, rolled + 100ms), nullptr);
		EXPECT_EQ(popAt(schedule, rolled + 100ms), nullptr);

		EXPECT_EQ(schedule.effectiveReserve(rolled + 300ms), 1u);
		for (auto n = 0; n < 5; ++n) {
			EXPECT_NE(popAt(schedule, rolled + 300ms), nullptr);
		}
		EXPECT_EQ(popAt(schedule, rolled + 300ms), nullptr);
		EXPECT_EQ(schedule.reserve(), 6u);
	}

	TEST(LanePolicy, FixedReserveIsNeverStale) {
		Schedule schedule;
		schedule.setup(8u, fixedReserve(4u));
		schedule.rollFrame();
		EXPECT_EQ(schedule.effectiveReserve(clock::now() + 10s), 4u);
	}

	TEST(LanePolicy, ResetReserveReturnsToTheMinimum) {
		Schedule schedule;
		schedule.setup(8u, LanePolicyConfig {});
		runFrame(schedule, 6u);
		ASSERT_EQ(schedule.reserve(), 6u);

		schedule.resetReserve();
		EXPECT_EQ(schedule.reserve(), 1u);
		for (auto n = 0; n < 8; ++n) {
			schedule.rollFrame();
			EXPECT_EQ(schedule.reserve(), 1u);
		}
	}

	TEST(LanePolicy, FrameWaitRaisesReserveAtOnce) {
		Schedule schedule;
		schedule.setup(8u, LanePolicyConfig {});
		ASSERT_EQ(schedule.reserve(), 1u);

		const auto b = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b));
		ASSERT_EQ(popAt(schedule, clock::now()), b);

		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(f));
		EXPECT_EQ(popAt(schedule, f->queuedAt + 300us), f);
		EXPECT_EQ(schedule.reserve(), 2u);
		EXPECT_GE(schedule.frameWaitMaxMicros(), 300uLL);
		schedule.leave(Lane::eFrame);

		for (auto n = 0; n < 8; ++n) {
			schedule.rollFrame();
			EXPECT_EQ(schedule.reserve(), 2u);
		}
		EXPECT_EQ(schedule.frameWaitMaxMicros(), 0uLL);
	}

	TEST(LanePolicy, FrameWaitRaiseRefreshesTheStaleClock) {
		Schedule schedule;
		schedule.setup(8u, LanePolicyConfig {});
		schedule.rollFrame();
		const auto rolled = clock::now();

		const auto b = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b));
		ASSERT_EQ(popAt(schedule, rolled), b);

		// a single frame longer than reserveStaleMillis: its raise must survive the staleness rule
		const auto late = rolled + 300ms;
		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(f));
		EXPECT_EQ(popAt(schedule, late), f);
		EXPECT_EQ(schedule.reserve(), 2u);
		EXPECT_EQ(schedule.effectiveReserve(late + 1ms), 2u);
	}

	TEST(LanePolicy, FrameWaitWithoutLowerWorkDoesNotRaise) {
		Schedule schedule;
		schedule.setup(8u, LanePolicyConfig {});

		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(f));
		EXPECT_EQ(popAt(schedule, f->queuedAt + 300us), f);
		EXPECT_EQ(schedule.reserve(), 1u);
	}

	TEST(LanePolicy, QueuedPerLane) {
		Schedule schedule;
		schedule.setup(4u, LanePolicyConfig {});

		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		for (auto n = 0; n < 2; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		}
		for (auto n = 0; n < 3; ++n) {
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		}
		EXPECT_EQ(schedule.queued(Lane::eFrame), 1uLL);
		EXPECT_EQ(schedule.queued(Lane::eInteractive), 2uLL);
		EXPECT_EQ(schedule.queued(Lane::eBackground), 3uLL);
		EXPECT_EQ(schedule.queued(), 6uLL);

		ASSERT_NE(popAt(schedule, clock::time_point {}), nullptr);
		EXPECT_EQ(schedule.queued(Lane::eFrame), 0uLL);
		EXPECT_EQ(schedule.queued(), 5uLL);
	}

	TEST(LanePolicy, ReservedWorkerParksAndFloorStillServes) {
		LanePolicyConfig config = fixedReserve(2u);
		config.backgroundFloorMicros = 5'000u;

		std::atomic_bool stop { false };
		std::atomic_bool firstRunning { false };
		std::atomic_bool firstDone { false };
		std::atomic_bool secondStarted { false };

		structuredRuntimeTest(
			3u,
			config,
			[&](nmpt<engine::Scheduler> s_) {
				const auto schedule = scheduleOf(s_);
				s_->exec(
					task::make_task(
						[&] {
							firstRunning = true;
							while (not stop.load()) {}
							firstDone = true;
						},
						Lane::eBackground
					)
				);
				ASSERT_TRUE(pollFor(2000ms, [&] { return firstRunning.load(); }));

				s_->exec(task::make_task([&] { secondStarted = true; }, Lane::eBackground));

				// no further pushes: only the bounded park brings a worker back to pop once the floor falls due
				const auto cpu0 = processCpuTime();
				const auto t0 = clock::now();
				std::ignore = pollFor(50ms, [&] { return secondStarted.load(); });
				const auto took = clock::now() - t0;
				const auto cpu = processCpuTime() - cpu0;
				std::printf(
					"[ lane     ] floor served after %lld us, process cpu %lld ms\n",
					static_cast<long long>(std::chrono::duration_cast<std::chrono::microseconds>(took).count()),
					static_cast<long long>(cpu.count())
				);
				EXPECT_TRUE(secondStarted.load());

				stop = true;
				EXPECT_TRUE(pollFor(2000ms, [&] { return firstDone.load() && schedule->queued() == 0uLL; }));
			}
		);
	}

	TEST(LanePolicy, ShrinkingReserveWakesParkedWorkers) {
		std::atomic_bool release { false };
		std::atomic<int> burstDone { 0 };
		std::atomic<int> started { 0 };
		std::atomic<int> done { 0 };
		std::atomic<clock::rep> firstEnd { 0 };
		std::atomic<clock::rep> lastStart { 0 };

		structuredRuntimeTest(
			4u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				const auto schedule = scheduleOf(s_);

				// a Frame burst held open, so the peak counts all three
				for (auto n = 0; n < 3; ++n) {
					ASSERT_TRUE(
						schedule->push(
							task::make_task(
								[&] {
									while (not release.load()) {}
									burstDone.fetch_add(1);
								},
								Lane::eFrame
							)
						)
					);
				}
				release = true;
				ASSERT_TRUE(pollFor(2000ms, [&] { return burstDone.load() == 3; }));
				schedule->rollFrame();
				ASSERT_EQ(schedule->reserve(), 3u);

				for (auto n = 0; n < 3; ++n) {
					s_->exec(
						task::make_task(
							[&] {
								const auto at = clock::now().time_since_epoch().count();
								if (started.fetch_add(1) == 2) {
									lastStart = at;
								}
								spinFor(20ms);
								auto expected = clock::rep { 0 };
								firstEnd.compare_exchange_strong(expected, clock::now().time_since_epoch().count());
								done.fetch_add(1);
							},
							Lane::eBackground
						)
					);
				}
				ASSERT_TRUE(pollFor(2000ms, [&] { return started.load() >= 1; }));
				thread::self::sleepFor(5);
				EXPECT_EQ(started.load(), 1);

				const auto cpu0 = processCpuTime();
				for (auto n = 0; n < 8; ++n) {
					schedule->rollFrame();
				}
				const auto lastRoll = clock::now();
				EXPECT_EQ(schedule->reserve(), 1u);
				EXPECT_TRUE(pollFor(50ms, [&] { return started.load() == 3; }));
				EXPECT_LE(lastStart.load() - lastRoll.time_since_epoch().count(), clock::duration { 50ms }.count());
				// the wake, not the first task's end, started the others
				EXPECT_LT(lastStart.load(), firstEnd.load() == 0 ? std::numeric_limits<clock::rep>::max() : firstEnd.load());

				EXPECT_TRUE(pollFor(2000ms, [&] { return done.load() == 3; }));
				std::printf(
					"[ lane     ] process cpu after the shrink %lld ms\n",
					static_cast<long long>((processCpuTime() - cpu0).count())
				);
			}
		);
	}

	TEST(LanePolicy, FanOutStartsDespiteStuckLowerLanes) {
		constexpr auto lowerTasks = 6;

		std::atomic_bool release { false };
		std::atomic<int> burstDone { 0 };
		std::atomic<int> lowerStarted { 0 };
		std::atomic<int> lowerDone { 0 };
		std::atomic<int> childrenStarted { 0 };
		std::atomic_bool rendezvousMissed { false };
		std::atomic<clock::rep> fanOut { 0 };
		std::atomic<clock::rep> childMax { 0 };

		structuredRuntimeTest(
			8u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				const auto schedule = scheduleOf(s_);

				for (auto n = 0; n < 4; ++n) {
					ASSERT_TRUE(
						schedule->push(
							task::make_task(
								[&] {
									while (not release.load()) {}
									burstDone.fetch_add(1);
								},
								Lane::eFrame
							)
						)
					);
				}
				release = true;
				ASSERT_TRUE(pollFor(2000ms, [&] { return burstDone.load() == 4; }));
				schedule->rollFrame();
				ASSERT_EQ(schedule->reserve(), 4u);

				const auto cpu0 = processCpuTime();
				for (auto n = 0; n < lowerTasks; ++n) {
					s_->exec(
						task::make_task(
							[&] {
								lowerStarted.fetch_add(1);
								spinFor(200ms);
								lowerDone.fetch_add(1);
							},
							Lane::eBackground
						)
					);
				}
				ASSERT_TRUE(pollFor(2000ms, [&] { return lowerStarted.load() >= 4; }));
				thread::self::sleepFor(5);
				EXPECT_EQ(lowerStarted.load(), 4);

				auto parent = task::make_task(
					[&, s = s_.get()] {
						fanOut = clock::now().time_since_epoch().count();
						for (auto n = 0; n < 4; ++n) {
							s->exec(
								task::make_task(
									[&] {
										const auto at = clock::now().time_since_epoch().count() - fanOut.load();
										auto seen = childMax.load();
										while (seen < at && not childMax.compare_exchange_weak(seen, at)) {}
										childrenStarted.fetch_add(1);
										// rendezvous: all four must run at once, so one worker cannot serve them in turn
										const auto deadline = clock::now() + 2000ms;
										while (childrenStarted.load() < 4 && clock::now() < deadline) {}
										if (childrenStarted.load() < 4) {
											rendezvousMissed = true;
										}
									}
								)
							);
						}
					},
					Lane::eFrame
				);
				ASSERT_TRUE(schedule->push(parent));
				EXPECT_TRUE(pollFor(2000ms, [&] { return childrenStarted.load() == 4; }));
				const auto pickup = std::chrono::duration_cast<std::chrono::microseconds>(clock::duration { childMax.load() });
				EXPECT_LE(pickup, 5ms);
				EXPECT_FALSE(rendezvousMissed.load());

				EXPECT_TRUE(pollFor(5000ms, [&] { return lowerDone.load() == lowerTasks; }));
				std::printf(
					"[ lane     ] fan-out pickup %lld us; process cpu %lld ms for %d x 200 ms lower spin\n",
					static_cast<long long>(pickup.count()),
					static_cast<long long>((processCpuTime() - cpu0).count()),
					lowerTasks
				);
			}
		);
	}

	TEST(LanePolicy, TakeableSeesFrameWorkReserveAndFloor) {
		{
			Schedule schedule {};
			schedule.setup(3u, fixedReserve(2u));
			const auto now = ScheduleTestAccess::setupStamp(schedule);

			EXPECT_FALSE(schedule.takeable(now));
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eFrame);
			EXPECT_TRUE(schedule.takeable(now));
		}
		{
			Schedule schedule {};
			schedule.setup(3u, fixedReserve(2u));
			const auto now = ScheduleTestAccess::setupStamp(schedule);

			// one background task running fills the single non-reserved worker
			ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
			ASSERT_NE(popAt(schedule, now), nullptr);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eBackground);

			EXPECT_FALSE(schedule.takeable(now));
			EXPECT_TRUE(schedule.takeable(now + 100ms));
			schedule.leave(Lane::eBackground);
			EXPECT_TRUE(schedule.takeable(now));
		}
	}

	TEST(LanePolicy, WakeAfterSubmitWakesPerTaskOffWorkerAndOnceOnWorker) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		const auto wakesFor = [&schedule](const u32 pushed_) {
			const auto before = schedule.wakeCalls();
			schedule.wakeAfterSubmit(pushed_);
			return schedule.wakeCalls() - before;
		};

		EXPECT_EQ(wakesFor(0u), 0uLL);
		EXPECT_EQ(wakesFor(3u), 3uLL);

		// a worker pops one pushed task itself; the cascade spreads the rest
		thread::self::markWorker(true);
		EXPECT_EQ(wakesFor(0u), 0uLL);
		EXPECT_EQ(wakesFor(1u), 0uLL);
		EXPECT_EQ(wakesFor(2u), 1uLL);
		EXPECT_EQ(wakesFor(5u), 1uLL);
		thread::self::markWorker(false);
	}

	TEST(LanePolicy, ParkTimeoutIsForeverWhenEmpty) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		EXPECT_EQ(schedule.parkTimeoutMillis(clock::now()), parkForever);
	}

	TEST(LanePolicy, ParkTimeoutIsZeroWhileFramePushInFlight) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		// Frame depth without an insert is a push in flight: retry at once instead of parking (lost wakeup otherwise)
		ScheduleTestAccess::countWithoutInsert(schedule, Lane::eFrame);
		EXPECT_EQ(schedule.parkTimeoutMillis(clock::now()), 0u);
	}

	TEST(LanePolicy, ParkTimeoutIsTheTimeToTheEarliestFloor) {
		LanePolicyConfig config {};
		config.interactiveFloorMicros = 5000uL;
		config.backgroundFloorMicros = 100000uL;
		const auto now = clock::now();

		{
			// due now: clamped up to one millisecond
			Schedule schedule {};
			schedule.setup(4u, config);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eInteractive);
			ScheduleTestAccess::setLastStart(schedule, 0uLL, now - 5000us);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 1u);
			// overdue is clamped the same way
			ScheduleTestAccess::setLastStart(schedule, 0uLL, now - 50ms);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 1u);
		}
		{
			Schedule schedule {};
			schedule.setup(4u, config);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eInteractive);
			ScheduleTestAccess::setLastStart(schedule, 0uLL, now);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 5u);
			// rounds up
			EXPECT_EQ(schedule.parkTimeoutMillis(now + 1500us), 4u);
		}
		{
			Schedule schedule {};
			schedule.setup(4u, config);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eBackground);
			ScheduleTestAccess::setLastStart(schedule, 1uLL, now);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 100u);
		}
		{
			// both queued: the smaller wait wins, whichever lane it is
			Schedule schedule {};
			schedule.setup(4u, config);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eInteractive);
			ScheduleTestAccess::countWithoutInsert(schedule, Lane::eBackground);
			ScheduleTestAccess::setLastStart(schedule, 0uLL, now);
			ScheduleTestAccess::setLastStart(schedule, 1uLL, now - 98ms);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 2u);
			ScheduleTestAccess::setLastStart(schedule, 1uLL, now);
			EXPECT_EQ(schedule.parkTimeoutMillis(now), 5u);
		}
	}

	TEST(LanePolicy, WaitForWorkReturnsAtTheTimeoutWithoutAPush) {
		Schedule schedule {};
		schedule.setup(4u, LanePolicyConfig {});

		const auto seen = schedule.epoch();
		schedule.waitForWork(seen, 1u);
		EXPECT_EQ(schedule.epoch(), seen);
	}
}
