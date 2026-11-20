#include "pch.h"

/**/

#include <Engine.Scheduler/Lane.hpp>
#include <Engine.Scheduler/LanePolicyConfig.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "TestCompScheduler.hpp"
#include "TestSchedulerEngine.hpp"

import std;

using namespace hg::engine::scheduler;
using namespace hg;
using namespace std::chrono_literals;

namespace SchedulerModule {
	namespace {
		using clock = std::chrono::steady_clock;

		ptr<Schedule> sliceScheduleOf(nmpt<engine::Scheduler> s_) {
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

		// Spins 3 ms checkpointing every 100 us; the count of resumptions is the number of yields it took.
		u32 resumesOfCheckpointingTask(const Lane lane_) {
			std::atomic_bool done { false };
			std::atomic<u32> resumes { 0u };

			structuredRuntimeTest(
				1u,
				LanePolicyConfig {},
				[&](nmpt<engine::Scheduler> s_) {
					thread::self::sleepFor(20);
					ASSERT_TRUE(
						sliceScheduleOf(s_)->push(
							task::make_task(
								[&] {
									auto* const self = fiber::self::current();
									auto starts = 1u;
									auto lastStart = self->sliceStart;
									const auto until = clock::now() + 3ms;
									while (clock::now() < until) {
										spinFor(100us);
										fiber::self::checkpoint();
										if (self->sliceStart != lastStart) {
											lastStart = self->sliceStart;
											++starts;
										}
									}
									resumes = starts - 1u;
									done = true;
								},
								lane_
							)
						)
					);
					EXPECT_TRUE(waitUntil(2000ms, done));
				}
			);
			return resumes.load();
		}
	}

	TEST(LaneSlice, CheckpointYieldsAfterTheSliceInALowerLane) {
		EXPECT_GE(resumesOfCheckpointingTask(Lane::eBackground), 2u);
	}

	TEST(LaneSlice, CheckpointNeverYieldsInFrame) {
		EXPECT_EQ(resumesOfCheckpointingTask(Lane::eFrame), 0u);
	}

	TEST(LaneSlice, CheckpointOutsideAFiberIsANoOp) {
		fiber::self::checkpoint();
		SUCCEED();
	}

	TEST(LaneSlice, OverrunIsCounted) {
		std::atomic_bool bgDone { false };
		std::atomic_bool frameDone { false };
		std::atomic<clock::rep> bgEnd { 0 };
		std::atomic<clock::rep> frameStart { 0 };

		structuredRuntimeTest(
			1u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = sliceScheduleOf(s_);
				std::atomic_bool started { false };

				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								started = true;
								spinFor(5ms);
								bgEnd = clock::now().time_since_epoch().count();
								bgDone = true;
							},
							Lane::eBackground
						)
					)
				);
				ASSERT_TRUE(waitUntil(1000ms, started));
				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								frameStart = clock::now().time_since_epoch().count();
								frameDone = true;
							},
							Lane::eFrame
						)
					)
				);
				ASSERT_TRUE(waitUntil(2000ms, frameDone));
				ASSERT_TRUE(waitUntil(2000ms, bgDone));

				EXPECT_GE(frameStart.load(), bgEnd.load());
				// the worker notes the run after the fiber returns
				thread::self::sleepFor(20);
				EXPECT_EQ(schedule->overruns(Lane::eBackground), 1uLL);
				EXPECT_GE(schedule->maxRunMicros(Lane::eBackground), 5000uLL);
				EXPECT_EQ(schedule->overruns(Lane::eFrame), 0uLL);
			}
		);
	}

	TEST(LaneSlice, FrameStartsWithinASliceUnderLowerLoad) {
		std::atomic_bool stop { false };
		std::atomic_bool started { false };
		std::atomic_bool frameDone { false };
		std::atomic<clock::rep> pushedAt { 0 };
		std::atomic<clock::rep> startedAt { 0 };

		structuredRuntimeTest(
			1u,
			LanePolicyConfig {},
			[&](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(20);
				const auto schedule = sliceScheduleOf(s_);
				// a failed assert returns early; the endless task must still end or the runtime never stops
				struct StopOnExit {
					std::atomic_bool& flag;

					~StopOnExit() {
						flag = true;
					}
				} const stopOnExit { stop };

				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								started = true;
								while (not stop.load()) {
									spinFor(50us);
									fiber::self::checkpoint();
								}
							},
							Lane::eBackground
						)
					)
				);
				ASSERT_TRUE(waitUntil(1000ms, started));
				thread::self::sleepFor(5);

				pushedAt = clock::now().time_since_epoch().count();
				ASSERT_TRUE(
					schedule->push(
						task::make_task(
							[&] {
								startedAt = clock::now().time_since_epoch().count();
								frameDone = true;
							},
							Lane::eFrame
						)
					)
				);
				EXPECT_TRUE(waitUntil(2000ms, frameDone));
				stop = true;
				EXPECT_LE(clock::duration { startedAt.load() - pushedAt.load() }, 5ms);
			}
		);
	}

	TEST(LaneSlice, NoteRunCountsOnlyRunsBeyondTwoSlices) {
		LanePolicyConfig config {};
		config.sliceMicros = 1000uL;
		// the runtime brings up the logger an overrun reports to; the idle schedule has no task to run
		structuredRuntimeTest(
			1u,
			config,
			[](nmpt<engine::Scheduler> s_) {
				auto& schedule = *sliceScheduleOf(s_);

				schedule.noteRun(Lane::eInteractive, 500uLL);
				schedule.noteRun(Lane::eInteractive, 2000uLL);
				EXPECT_EQ(schedule.overruns(Lane::eInteractive), 0uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eInteractive), 2000uLL);

				schedule.noteRun(Lane::eInteractive, 2001uLL);
				EXPECT_EQ(schedule.overruns(Lane::eInteractive), 1uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eInteractive), 2001uLL);

				// a smaller overrun does not lower the max
				schedule.noteRun(Lane::eInteractive, 3000uLL);
				schedule.noteRun(Lane::eInteractive, 2500uLL);
				EXPECT_EQ(schedule.overruns(Lane::eInteractive), 3uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eInteractive), 3000uLL);

				// lanes are independent
				EXPECT_EQ(schedule.overruns(Lane::eBackground), 0uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eBackground), 0uLL);
				schedule.noteRun(Lane::eBackground, 100uLL);
				EXPECT_EQ(schedule.overruns(Lane::eBackground), 0uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eBackground), 100uLL);
				EXPECT_EQ(schedule.overruns(Lane::eInteractive), 3uLL);
				EXPECT_EQ(schedule.maxRunMicros(Lane::eInteractive), 3000uLL);
			}
		);
	}
}
