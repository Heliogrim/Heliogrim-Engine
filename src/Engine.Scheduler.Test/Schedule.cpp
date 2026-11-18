#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <Engine.Scheduler/Lane.hpp>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	static non_owning_rptr<const task::TaskDelegate> makeTask(const Lane lane_) {
		return task::make_task([] {}, lane_);
	}

	TEST(LaneSchedule, PopsFrameThenInteractiveThenBackground) {
		Schedule schedule;
		schedule.setup();

		const auto b = makeTask(Lane::eBackground);
		const auto i = makeTask(Lane::eInteractive);
		const auto f = makeTask(Lane::eFrame);
		ASSERT_TRUE(schedule.push(b));
		ASSERT_TRUE(schedule.push(i));
		ASSERT_TRUE(schedule.push(f));

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, f);
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, i);
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, b);
	}

	TEST(LaneSchedule, FifoInsideALane) {
		Schedule schedule;
		schedule.setup();

		const auto first = makeTask(Lane::eInteractive);
		const auto second = makeTask(Lane::eInteractive);
		ASSERT_TRUE(schedule.push(first));
		ASSERT_TRUE(schedule.push(second));

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, first);
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, second);
	}

	TEST(LaneSchedule, NothingIsPromotedByTime) {
		Schedule schedule;
		schedule.setup();

		const auto b = makeTask(Lane::eBackground);
		ASSERT_TRUE(schedule.push(b));

		// no ageing: pushing and draining frame work repeatedly never moves the background task
		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		for (auto frame = 0; frame < 3; ++frame) {
			const auto f = makeTask(Lane::eFrame);
			ASSERT_TRUE(schedule.push(f));
			ASSERT_TRUE(schedule.pop(out));
			EXPECT_EQ(out, f);
		}

		EXPECT_EQ(b->lane(), Lane::eBackground);
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(out, b);
	}

	TEST(LaneSchedule, QueuedCountsEveryLane) {
		Schedule schedule;
		schedule.setup();

		ASSERT_TRUE(schedule.push(makeTask(Lane::eFrame)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		ASSERT_TRUE(schedule.push(makeTask(Lane::eBackground)));
		EXPECT_EQ(schedule.queued(), 3uLL);

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		for (auto n = 0; n < 3; ++n) {
			ASSERT_TRUE(schedule.pop(out));
		}
		EXPECT_EQ(schedule.queued(), 0uLL);
	}

	TEST(Schedule, PushBumpsEpochAndQueued) {
		Schedule schedule;
		schedule.setup();
		const auto e0 = schedule.epoch();
		ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		EXPECT_NE(schedule.epoch(), e0);
		EXPECT_EQ(schedule.queued(), 1);
		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		ASSERT_TRUE(schedule.pop(out));
		EXPECT_EQ(schedule.queued(), 0);
	}

	TEST(Schedule, WaitForWorkReturnsOnPushFromPlainThread) {
		Schedule schedule;
		schedule.setup();
		std::atomic<bool> returned { false };
		const auto seen = schedule.epoch();
		std::thread waiter {
			[&] {
				while (schedule.epoch() == seen) {
					schedule.waitForWork(seen);
				}
				returned = true;
			}
		};
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		EXPECT_FALSE(returned.load());
		ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		waiter.join();
		EXPECT_TRUE(returned.load());
	}

	TEST(Schedule, WaitForWorkReturnsAtOnceWhenEpochMoved) {
		Schedule schedule;
		schedule.setup();
		const auto seen = schedule.epoch();
		ASSERT_TRUE(schedule.push(makeTask(Lane::eInteractive)));
		schedule.waitForWork(seen);
		SUCCEED();
	}

	TEST(Schedule, WakeAllReleasesEveryWaiter) {
		Schedule schedule;
		schedule.setup();
		const auto seen = schedule.epoch();
		std::vector<std::thread> waiters;
		for (int i = 0; i < 4; ++i) {
			waiters.emplace_back(
				[&] {
					while (schedule.epoch() == seen) {
						schedule.waitForWork(seen);
					}
				}
			);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		schedule.wakeAll();
		for (auto& t : waiters) {
			t.join();
		}
		SUCCEED();
	}
}
