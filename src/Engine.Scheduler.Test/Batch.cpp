#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#include <Engine.Common/Algo/Search.hpp>
#include <Engine.Common/Algo/Slice.hpp>
#include <Engine.Scheduler/Lane.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Task/Batch.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "TestSchedulerEngine.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	namespace {
		constexpr auto deadline = std::chrono::seconds { 5 };

		template <typename Pred_>
		bool waitUntil(Pred_&& pred_, const std::chrono::steady_clock::duration limit_ = deadline) {
			const auto until = std::chrono::steady_clock::now() + limit_;
			while (not pred_() && std::chrono::steady_clock::now() < until) {
				thread::self::sleepFor(1);
			}
			return pred_();
		}

		void spinFor(const std::chrono::microseconds duration_) {
			const auto until = std::chrono::steady_clock::now() + duration_;
			while (std::chrono::steady_clock::now() < until) {}
		}

		[[nodiscard]] bool allHitOnce(ref<const std::vector<std::atomic<u32>>> hits_) {
			return FindIf(hits_, [](const auto& hit_) { return hit_.load() != 1uL; }) == hits_.end();
		}
	}

	TEST(Batch, EveryIndexRunsOnce) {
		std::vector<std::atomic<u32>> hits(1000uL);
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				auto handle = batch(
					*s_,
					BatchCount { 1000u },
					[&hits](const u16 index_, const u16 count_) {
						EXPECT_EQ(count_, 1000u);
						hits[index_].fetch_add(1uL);
					}
				);
				EXPECT_EQ(handle.size(), 1000u);
				ASSERT_TRUE(waitUntil([&] { return handle.isDone(); }));
				handle.wait();
			}
		);
		EXPECT_TRUE(allHitOnce(hits));
	}

	TEST(Batch, ZeroCountIsAutoAndEmptyDataGivesEmptySlices) {
		std::atomic<u32> calls { 0uL };
		std::atomic<u32> touched { 0uL };
		const std::vector<int> empty {};
		structuredRuntimeTest(
			2u,
			[&](nmpt<engine::Scheduler> s_) {
				parallelFor(
					*s_,
					BatchCount { 0u },
					[&](const u16 index_, const u16 count_) {
						calls.fetch_add(1uL);
						touched.fetch_add(static_cast<u32>(sliceOf(empty, index_, count_).size()));
					}
				);
			}
		);
		EXPECT_EQ(calls.load(), 2uL);
		EXPECT_EQ(touched.load(), 0uL);
	}

	TEST(Batch, DroppedHandleStillCompletes) {
		std::atomic<u32> counter { 0uL };
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				(void)batch(*s_, BatchCount { 256u }, [&counter](u16, u16) { counter.fetch_add(1uL); });
				EXPECT_TRUE(waitUntil([&] { return counter.load() == 256uL; }, std::chrono::seconds { 1 }));
			}
		);
		EXPECT_EQ(counter.load(), 256uL);
	}

	TEST(Batch, AwaitFromFiber) {
		std::vector<std::atomic<u32>> hits(64uL);
		std::atomic_bool seenAll { false };
		std::atomic_bool done { false };
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&, s = s_.get()] {
							auto handle = batch(*s, BatchCount { 64u }, [&hits](const u16 index_, u16) { hits[index_].fetch_add(1uL); });
							handle.wait();
							seenAll.store(allHitOnce(hits));
							done.store(true);
						}
					)
				);
				EXPECT_TRUE(waitUntil([&] { return done.load(); }));
			}
		);
		EXPECT_TRUE(seenAll.load());
	}

	TEST(Batch, ShardsUseManyWorkers) {
		std::mutex mtx {};
		std::set<std::thread::id> threads {};
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				auto handle = batch(
					*s_,
					BatchCount { 64u },
					[&](u16, u16) {
						spinFor(std::chrono::microseconds { 200 });
						const std::scoped_lock<std::mutex> lock { mtx };
						threads.insert(std::this_thread::get_id());
					}
				);
				ASSERT_TRUE(waitUntil([&] { return handle.isDone(); }));
				handle.wait();
			}
		);
		EXPECT_GT(threads.size(), 1uLL);
	}

	TEST(Batch, ParallelForCallerTakesPart) {
		std::mutex mtx {};
		std::set<std::thread::id> threads {};
		std::thread::id caller {};
		std::atomic_bool done { false };
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&, s = s_.get()] {
							caller = std::this_thread::get_id();
							parallelFor(
								*s,
								BatchCount { 64u },
								[&](u16, u16) {
									spinFor(std::chrono::microseconds { 200 });
									const std::scoped_lock<std::mutex> lock { mtx };
									threads.insert(std::this_thread::get_id());
								}
							);
							done.store(true);
						}
					)
				);
				EXPECT_TRUE(waitUntil([&] { return done.load(); }));
			}
		);
		ASSERT_TRUE(done.load());
		EXPECT_TRUE(threads.contains(caller));
	}

	TEST(Batch, ParallelForFromPlainThread) {
		std::vector<std::atomic<u32>> hits(500uL);
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				parallelFor(*s_, BatchCount { 500u }, [&hits](const u16 index_, u16) { hits[index_].fetch_add(1uL); });
				EXPECT_TRUE(allHitOnce(hits));
			}
		);
	}

	TEST(Batch, AutoCountIsTheWorkerCountAndSlicesCoverTheData) {
		std::vector<std::atomic<u32>> hits(1000uL);
		std::atomic<u32> parts { 0uL };
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				auto handle = batch(*s_, BatchCount::eAuto, [&](u16, u16) {});
				handle.wait();
				EXPECT_EQ(handle.size(), 4u);

				parallelFor(
					*s_,
					BatchCount::eAuto,
					[&](const u16 index_, const u16 count_) {
						EXPECT_EQ(count_, 4u);
						parts.fetch_add(1uL);
						for (auto& hit : sliceOf(hits, index_, count_)) {
							hit.fetch_add(1uL);
						}
					}
				);
			}
		);
		EXPECT_EQ(parts.load(), 4uL);
		EXPECT_TRUE(allHitOnce(hits));
	}

	TEST(Batch, SingleWorkerRunsAllIndices) {
		std::vector<std::atomic<u32>> hits(100uL);
		std::atomic_bool done { false };
		structuredRuntimeTest(
			1u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&, s = s_.get()] {
							parallelFor(*s, BatchCount { 100u }, [&hits](const u16 index_, u16) { hits[index_].fetch_add(1uL); });
							done.store(true);
						}
					)
				);
				EXPECT_TRUE(waitUntil([&] { return done.load(); }));
			}
		);
		EXPECT_TRUE(allHitOnce(hits));
	}

	TEST(Batch, ShardsInheritTheLane) {
		std::atomic<u32> background { 0uL };
		std::atomic<u32> other { 0uL };
		std::atomic_bool done { false };
		structuredRuntimeTest(
			4u,
			[&](nmpt<engine::Scheduler> s_) {
				s_->exec(
					task::make_task(
						[&, s = s_.get()] {
							auto handle = batch(
								*s,
								BatchCount { 32u },
								[&](u16, u16) {
									auto& counter = fiber::self::current()->task->lane() == Lane::eBackground ?
										background :
										other;
									counter.fetch_add(1uL);
								}
							);
							handle.wait();
							done.store(true);
						},
						Lane::eBackground
					)
				);
				EXPECT_TRUE(waitUntil([&] { return done.load(); }));
			}
		);
		EXPECT_EQ(background.load(), 32uL);
		EXPECT_EQ(other.load(), 0uL);
	}
}
