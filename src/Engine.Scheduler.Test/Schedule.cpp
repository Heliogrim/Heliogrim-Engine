#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <Engine.Scheduler/Process/Schedule.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	static non_owning_rptr<const task::TaskDelegate> makeTask(const task::TaskMask mask_) {
		return task::make_task([] {}, mask_);
	}

	TEST(Schedule, PopWalksDownFromOwnPriority) {
		Schedule schedule;
		schedule.setup();

		const auto task = makeTask(task::TaskMask::eLower);
		ASSERT_TRUE(schedule.push(task));

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		EXPECT_TRUE(schedule.pop(task::TaskMask::eCritical, out));
		EXPECT_EQ(out, task);
	}

	TEST(Schedule, PopNeverWalksUp) {
		Schedule schedule;
		schedule.setup();

		const auto task = makeTask(task::TaskMask::eCritical);
		ASSERT_TRUE(schedule.push(task));

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		EXPECT_FALSE(schedule.pop(task::TaskMask::eLower, out));
		EXPECT_TRUE(schedule.pop(task::TaskMask::eCritical, out));
		EXPECT_EQ(out, task);
	}

	TEST(Schedule, TickPromotesEverySet) {
		// pop(mask) also reaches lower-priority sets, so the set just below the expected one must be empty.
		constexpr task::TaskMask expected[] {
			task::TaskMask::eLower,
			task::TaskMask::eNormal,
			task::TaskMask::eHigher,
			task::TaskMask::eCritical
		};

		for (u32 ticks = 0u; ticks < 4u; ++ticks) {
			Schedule schedule;
			schedule.setup();

			const auto task = makeTask(task::TaskMask::eLower);
			ASSERT_TRUE(schedule.push(task));

			for (u32 i = 0u; i < ticks; ++i) {
				schedule.tick();
			}

			non_owning_rptr<const task::TaskDelegate> out = nullptr;
			if (ticks > 0u) {
				EXPECT_FALSE(schedule.pop(expected[ticks - 1u], out)) << "ticks=" << ticks;
			}
			EXPECT_TRUE(schedule.pop(expected[ticks], out)) << "ticks=" << ticks;
			EXPECT_EQ(out, task);
		}
	}

	TEST(Schedule, TickKeepsFifoWithinSet) {
		Schedule schedule;
		schedule.setup();

		const auto a = makeTask(task::TaskMask::eNormal);
		const auto b = makeTask(task::TaskMask::eNormal);
		ASSERT_TRUE(schedule.push(a));
		ASSERT_TRUE(schedule.push(b));
		schedule.tick();

		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		EXPECT_TRUE(schedule.pop(task::TaskMask::eHigher, out));
		EXPECT_EQ(out, a);
		EXPECT_TRUE(schedule.pop(task::TaskMask::eHigher, out));
		EXPECT_EQ(out, b);
		EXPECT_FALSE(schedule.pop(task::TaskMask::eHigher, out));
	}

	TEST(Schedule, PushBumpsEpochAndQueued) {
		Schedule schedule;
		schedule.setup();
		const auto e0 = schedule.epoch();
		ASSERT_TRUE(schedule.push(makeTask(task::TaskMask::eNormal)));
		EXPECT_NE(schedule.epoch(), e0);
		EXPECT_EQ(schedule.queued(), 1);
		non_owning_rptr<const task::TaskDelegate> out = nullptr;
		ASSERT_TRUE(schedule.pop(task::TaskMask::eAll, out));
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
		ASSERT_TRUE(schedule.push(makeTask(task::TaskMask::eNormal)));
		waiter.join();
		EXPECT_TRUE(returned.load());
	}

	TEST(Schedule, WaitForWorkReturnsAtOnceWhenEpochMoved) {
		Schedule schedule;
		schedule.setup();
		const auto seen = schedule.epoch();
		ASSERT_TRUE(schedule.push(makeTask(task::TaskMask::eNormal)));
		schedule.waitForWork(seen);
		SUCCEED();
	}

	TEST(Schedule, TransferBumpsEpochOnlyWhenTasksMoved) {
		Schedule schedule;
		schedule.setup();
		const auto e0 = schedule.epoch();
		schedule.tick();
		EXPECT_EQ(schedule.epoch(), e0);
		ASSERT_TRUE(schedule.push(makeTask(task::TaskMask::eLower)));
		const auto e1 = schedule.epoch();
		schedule.tick();
		EXPECT_NE(schedule.epoch(), e1);
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
