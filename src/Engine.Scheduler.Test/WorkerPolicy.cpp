#include "pch.h"

/**/

#include <Engine.Scheduler/CompScheduler.hpp>

namespace SchedulerModule {
	TEST(WorkerPolicy, AutoWorkerCountTable) {
		using hg::engine::scheduler::autoWorkerCount;
		EXPECT_EQ(autoWorkerCount(1u, 0u), 1u);
		EXPECT_EQ(autoWorkerCount(2u, 0u), 1u);
		EXPECT_EQ(autoWorkerCount(4u, 0u), 2u);
		EXPECT_EQ(autoWorkerCount(8u, 0u), 5u);
		EXPECT_EQ(autoWorkerCount(16u, 0u), 12u);
		EXPECT_EQ(autoWorkerCount(24u, 0u), 20u);
		EXPECT_EQ(autoWorkerCount(32u, 0u), 27u);
		EXPECT_EQ(autoWorkerCount(64u, 0u), 58u);
		EXPECT_EQ(autoWorkerCount(24u, 8u), 16u);// explicit reserve
		EXPECT_EQ(autoWorkerCount(4u, 16u), 1u);// reserve above logical floors at 1
	}
}
