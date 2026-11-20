#include "pch.h"

/**/

#include <Engine.Scheduler/Lane.hpp>
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

namespace SchedulerModule {
	namespace {
		[[nodiscard]] Lane runningLane() {
			return fiber::self::current()->task->lane();
		}

		bool waitFor(ref<const std::atomic_bool> flag_) {
			const auto until = std::chrono::steady_clock::now() + std::chrono::seconds { 2 };
			while (not flag_.load() && std::chrono::steady_clock::now() < until) {
				thread::self::sleepFor(1);
			}
			return flag_.load();
		}

		// Runs parent_ as a task of lane_ and reports the lane child_ resolved to when the parent execs it.
		[[nodiscard]] Lane childLane(const Lane parent_, const Lane request_) {
			std::atomic<Lane> seen { Lane::eInherit };
			std::atomic_bool done { false };

			structuredRuntimeTest(
				2u,
				[&](nmpt<engine::Scheduler> s_) {
					auto parent = task::make_task(
						[&seen, &done, s = s_.get(), request_] {
							s->exec(
								task::make_task(
									[&seen, &done] {
										seen.store(runningLane());
										done.store(true);
									},
									request_
								)
							);
						},
						parent_
					);
					// pushed like a worker or stage would, never resolved
					static_cast<ptr<engine::TestCompScheduler>>(s_.get())->testGetSchedule()->push(parent);
					EXPECT_TRUE(waitFor(done));
				}
			);
			return seen.load();
		}
	}

	TEST(LaneResolution, PlainThreadDefaultsToInteractive) {
		std::atomic<Lane> seen { Lane::eInherit };
		std::atomic_bool done { false };
		structuredRuntimeTest(
			2u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&] {
							seen.store(runningLane());
							done.store(true);
						}
					)
				);
				EXPECT_TRUE(waitFor(done));
			}
		);
		EXPECT_EQ(seen.load(), Lane::eInteractive);
	}

	TEST(LaneResolution, PlainThreadCannotRequestFrame) {
		std::atomic<Lane> seen { Lane::eInherit };
		std::atomic_bool done { false };
		structuredRuntimeTest(
			2u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&] {
							seen.store(runningLane());
							done.store(true);
						},
						Lane::eFrame
					)
				);
				EXPECT_TRUE(waitFor(done));
			}
		);
		EXPECT_EQ(seen.load(), Lane::eInteractive);
	}

	TEST(LaneResolution, ChildInheritsParentLane) {
		EXPECT_EQ(childLane(Lane::eBackground, Lane::eInherit), Lane::eBackground);
	}

	TEST(LaneResolution, ChildMayGoLowerNotHigher) {
		EXPECT_EQ(childLane(Lane::eInteractive, Lane::eBackground), Lane::eBackground);
		EXPECT_EQ(childLane(Lane::eInteractive, Lane::eFrame), Lane::eInteractive);
	}

	TEST(LaneResolution, FrameStageChildIsFrame) {
		EXPECT_EQ(childLane(Lane::eFrame, Lane::eInherit), Lane::eFrame);
	}

	TEST(LaneResolution, HigherPicksTheNumericallyLowerLane) {
		EXPECT_EQ(higher(Lane::eFrame, Lane::eBackground), Lane::eFrame);
		EXPECT_EQ(higher(Lane::eBackground, Lane::eInteractive), Lane::eInteractive);
		EXPECT_EQ(higher(Lane::eInteractive, Lane::eFrame), Lane::eFrame);

		for (const auto a : { Lane::eFrame, Lane::eInteractive, Lane::eBackground }) {
			EXPECT_EQ(higher(a, a), a);
			for (const auto b : { Lane::eFrame, Lane::eInteractive, Lane::eBackground }) {
				EXPECT_EQ(higher(a, b), higher(b, a));
			}
		}
	}
}
