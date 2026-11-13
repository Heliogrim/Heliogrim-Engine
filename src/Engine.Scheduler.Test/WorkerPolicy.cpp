#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <set>
#include <thread>
#include <Engine.Scheduler/CompScheduler.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Waitable/Signal.hpp>

#include "TestSchedulerEngine.hpp"

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <processthreadsapi.h>
#endif

using namespace hg::engine::scheduler;
using namespace hg;

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

	namespace {
		using clock = std::chrono::steady_clock;

		// Polls pred_ with 1 ms sleeps; true when it held within limit_.
		bool pollFor(const std::chrono::milliseconds limit_, std::function<bool()>&& pred_) {
			const auto until = clock::now() + limit_;
			while (not pred_()) {
				if (clock::now() > until) {
					return pred_();
				}
				thread::self::sleepFor(1);
			}
			return true;
		}

		#ifdef WIN32
		std::chrono::milliseconds processCpuTime() {
			FILETIME creation {}, exit {}, kernel {}, user {};
			::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel, &user);
			const auto ticks = [](ref<const FILETIME> ft_) { return (static_cast<u64>(ft_.dwHighDateTime) << 32uLL) | ft_.dwLowDateTime; };
			// FILETIME counts 100 ns
			return std::chrono::milliseconds { (ticks(kernel) + ticks(user)) / 10'000uLL };
		}
		#endif
	}

	#ifdef WIN32
	TEST(WorkerPolicy, IdleWorkersParkCpuTime) {
		structuredRuntimeTest(
			4u,
			[](nmpt<engine::Scheduler>) {
				thread::self::sleepFor(200);
				const auto before = processCpuTime();
				thread::self::sleepFor(500);
				const auto delta = processCpuTime() - before;
				EXPECT_LT(delta.count(), 50) << "idle workers burned " << delta.count() << " ms CPU in 500 ms";
			}
		);
	}
	#endif

	TEST(WorkerPolicy, ExecWakesParkedWorker) {
		structuredRuntimeTest(
			4u,
			[](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(100);
				std::atomic_flag done {};
				s_->exec(task::make_task([&] { done.test_and_set(); }));
				EXPECT_TRUE(pollFor(std::chrono::milliseconds { 100 }, [&] { return done.test(); }));
				ASSERT_TRUE(pollFor(std::chrono::seconds { 2 }, [&] { return done.test(); }));
			}
		);
	}

	TEST(WorkerPolicy, BurstFromPlainThreadCompletes) {
		structuredRuntimeTest(
			4u,
			[](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(100);
				std::atomic<u32> count { 0u };
				for (u32 i = 0; i < 64u; ++i) {
					s_->exec(task::make_task([&] { count.fetch_add(1u); }));
				}
				EXPECT_TRUE(pollFor(std::chrono::seconds { 1 }, [&] { return count.load() == 64u; }));
				ASSERT_TRUE(pollFor(std::chrono::seconds { 5 }, [&] { return count.load() == 64u; }));
			}
		);
	}

	TEST(WorkerPolicy, FanOutFromWorkerReachesParkedPeers) {
		structuredRuntimeTest(
			4u,
			[](nmpt<engine::Scheduler> s_) {
				thread::self::sleepFor(100);
				std::atomic<u32> count { 0u };
				std::mutex lock {};
				std::set<thread::thread_id> ids {};

				s_->exec(
					task::make_task(
						[&] {
							for (u32 i = 0; i < 32u; ++i) {
								s_->exec(
									task::make_task(
										[&] {
											{
												const std::scoped_lock<std::mutex> guard { lock };
												ids.insert(thread::self::getId());
											}
											const auto until = clock::now() + std::chrono::milliseconds { 2 };
											while (clock::now() < until) {}
											count.fetch_add(1u);
										}
									)
								);
							}
						}
					)
				);

				EXPECT_TRUE(pollFor(std::chrono::seconds { 2 }, [&] { return count.load() == 32u; }));
				ASSERT_TRUE(pollFor(std::chrono::seconds { 5 }, [&] { return count.load() == 32u; }));
				const std::scoped_lock<std::mutex> guard { lock };
				EXPECT_GE(ids.size(), 2uLL);
			}
		);
	}

	TEST(WorkerPolicy, UnparkFromPlainThreadWakesWorker) {
		structuredRuntimeTest(
			2u,
			[](nmpt<engine::Scheduler> s_) {
				Signal event {};
				std::atomic_flag done {};
				s_->exec(
					task::make_task(
						[&] {
							event.wait();
							done.test_and_set();
						}
					)
				);
				thread::self::sleepFor(100);
				EXPECT_FALSE(done.test());
				event.set();
				EXPECT_TRUE(pollFor(std::chrono::milliseconds { 100 }, [&] { return done.test(); }));
				ASSERT_TRUE(pollFor(std::chrono::seconds { 2 }, [&] { return done.test(); }));
			}
		);
	}

	TEST(WorkerPolicy, DestroyWithParkedWorkersReturns) {
		structuredRuntimeTest(8u, [](nmpt<engine::Scheduler>) { thread::self::sleepFor(100); });
	}

	TEST(WorkerPolicy, StressBurstsWithParking) {
		structuredRuntimeTest(
			8u,
			[](nmpt<engine::Scheduler> s_) {
				std::atomic<u32> count { 0u };
				const auto burst = [&] {
					for (u32 i = 0; i < 5'000u; ++i) {
						s_->exec(task::make_task([&] { count.fetch_add(1u, std::memory_order::relaxed); }));
					}
				};

				for (u32 round = 0; round < 50u; ++round) {
					count.store(0u);
					std::thread a { burst };
					std::thread b { burst };
					a.join();
					b.join();
					ASSERT_TRUE(pollFor(std::chrono::seconds { 5 }, [&] { return count.load() == 10'000u; }))
						<< "round " << round << " stalled at " << count.load();
					thread::self::sleepFor(5);
				}
			}
		);
	}
}
