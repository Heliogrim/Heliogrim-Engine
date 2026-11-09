#include "pch.h"

/**/

#include <atomic>
#include <memory>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Cancellation.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Waitable/Event.hpp>
#include <Heliogrim/Async/Await.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(Cancellation, FalseOnPlainThread) {
		EXPECT_FALSE(fiber::self::cancellationRequested());
	}

	TEST(Cancellation, TokenVisibleInsideTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				struct State {
					::hg::Signal event {};
					std::atomic_bool first { true };
					std::atomic_bool second { false };
					std::atomic_bool reached { false };
					std::atomic_bool done { false };
				};

				auto state = std::make_shared<State>();
				auto source = Arci<fiber::CancellationSource>::create();
				fiber::CancellationToken token { source };

				s_->exec(
					task::make_task(
						[state] {
							state->first = fiber::self::cancellationRequested();
							state->reached = true;
							::hg::await(state->event);
							state->second = fiber::self::cancellationRequested();
							state->done = true;
						}
					),
					token
				);

				for (u32 i = 0; i < 2000 && not state->reached.load(); ++i) {
					thread::self::sleepFor(1);
				}
				source->cancel();
				state->event.set();

				for (u32 i = 0; i < 2000 && not state->done.load(); ++i) {
					thread::self::sleepFor(1);
				}
				EXPECT_TRUE(state->done.load());
				EXPECT_FALSE(state->first.load());
				EXPECT_TRUE(state->second.load());
			}
		);
	}
}
