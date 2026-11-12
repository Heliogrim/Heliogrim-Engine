#include "pch.h"

/**/

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
}
