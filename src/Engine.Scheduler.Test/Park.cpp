#include "pch.h"

/**/

#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(FiberSelf, CurrentIsNullOnPlainThread) {
		EXPECT_EQ(fiber::self::current(), nullptr);
	}

	TEST(FiberSelf, CurrentIsSetInsideTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				std::atomic<ptr<fiber::Fiber>> seen { nullptr };
				std::atomic_flag done {};
				s_->exec(
					task::make_task(
						[&] {
							seen = fiber::self::current();
							done.test_and_set();
							done.notify_one();
						}
					)
				);
				done.wait(false);
				EXPECT_NE(seen.load(), nullptr);
			}
		);
	}
}
