#include "pch.h"

/**/

#include <atomic>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Fiber/Park.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
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

	static std::atomic<ptr<fiber::Fiber>> parkingFiber { nullptr };

	static void waitFlag(ref<std::atomic_flag> flag_) {
		for (u32 i = 0; i < 2000 && not flag_.test(); ++i) {
			thread::self::sleepFor(1);
		}
	}

	TEST(Park, FiberParksAndThreadUnparks) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				std::atomic<ptr<ParkNode>> node { nullptr };
				std::atomic_flag done {};
				s_->exec(
					task::make_task(
						[&] {
							auto& n = currentParkNode();
							n.state = ParkNode::State::eArmed;
							node = &n;
							park(n);
							done.test_and_set();
							done.notify_one();
						}
					)
				);
				for (u32 i = 0; i < 2000 && node.load() == nullptr; ++i) {
					thread::self::sleepFor(1);
				}
				ASSERT_NE(node.load(), nullptr);
				thread::self::sleepFor(10);
				unpark(*node.load());
				waitFlag(done);
				EXPECT_TRUE(done.test());
			}
		);
	}

	#ifdef _DEBUG
	TEST(Park, UnparkBeforeWorkerParkedRunsOnce) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				std::atomic_int counter { 0 };
				std::atomic_flag done {};
				parkingFiber = nullptr;
				worker::debugPostSwitchHook = [](ptr<fiber::Fiber> fiber_) {
					auto expected = fiber_;
					if (parkingFiber.compare_exchange_strong(expected, nullptr)) {
						unpark(fiber_->parkNode);
					}
				};
				s_->exec(
					task::make_task(
						[&] {
							auto& n = currentParkNode();
							parkingFiber = fiber::self::current();
							n.state = ParkNode::State::eArmed;
							park(n);
							++counter;
							done.test_and_set();
							done.notify_one();
						}
					)
				);
				waitFlag(done);
				worker::debugPostSwitchHook = nullptr;
				ASSERT_TRUE(done.test());
				thread::self::sleepFor(10);
				EXPECT_EQ(counter.load(), 1);
			}
		);
	}
	#endif

	TEST(Park, UnparkAfterWorkerParkedRunsOnce) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				std::atomic_int counter { 0 };
				std::atomic_flag done {};
				parkingFiber = nullptr;
				s_->exec(
					task::make_task(
						[&] {
							auto& n = currentParkNode();
							n.state = ParkNode::State::eArmed;
							parkingFiber = fiber::self::current();
							park(n);
							++counter;
							done.test_and_set();
							done.notify_one();
						}
					)
				);
				for (u32 i = 0; i < 2000 && parkingFiber.load() == nullptr; ++i) {
					thread::self::sleepFor(1);
				}
				ASSERT_NE(parkingFiber.load(), nullptr);
				thread::self::sleepFor(10);
				unpark(parkingFiber.load()->parkNode);
				waitFlag(done);
				ASSERT_TRUE(done.test());
				thread::self::sleepFor(10);
				EXPECT_EQ(counter.load(), 1);
			}
		);
	}

	TEST(Park, PlainYieldStillReschedules) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				std::atomic_flag done {};
				s_->exec(
					task::make_task(
						[&] {
							for (u32 i = 0; i < 100; ++i) {
								fiber::self::yield();
							}
							done.test_and_set();
							done.notify_one();
						}
					)
				);
				waitFlag(done);
				EXPECT_TRUE(done.test());
			}
		);
	}
}
