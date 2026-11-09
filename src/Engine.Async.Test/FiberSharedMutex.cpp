#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <Engine.Async/Primitive/FiberSharedMutex.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Heliogrim/Async/Yield.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;

namespace AsyncModule {
	namespace {
		template <typename Pred_>
		bool waitFor(Pred_&& pred_, const u32 ms_) {
			for (u32 i = 0; i < ms_ && not pred_(); ++i) {
				engine::scheduler::thread::self::sleepFor(1);
			}
			return pred_();
		}

		// Fiber-side wait: yields instead of blocking the worker.
		template <typename Pred_>
		bool yieldUntil(Pred_&& pred_, const u32 ms_) {
			const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms_);
			while (not pred_() && std::chrono::steady_clock::now() < deadline) {
				::hg::yield();
			}
			return pred_();
		}

		// try_lock_shared fails exactly while a writer holds or waits.
		bool writerPresent(ref<concurrent::FiberSharedMutex> mtx_) {
			if (mtx_.try_lock_shared()) {
				mtx_.unlock_shared();
				return false;
			}
			return true;
		}
	}

	TEST(FiberSharedMutex, SharedLockCompiles) {
		concurrent::FiberSharedMutex mtx {}; {
			std::shared_lock<concurrent::FiberSharedMutex> guard { mtx };
			EXPECT_FALSE(mtx.try_lock());
		} {
			std::unique_lock<concurrent::FiberSharedMutex> guard { mtx };
			EXPECT_FALSE(mtx.try_lock_shared());
		}
		EXPECT_TRUE(mtx.try_lock());
		mtx.unlock();
	}

	TEST(FiberSharedMutex, ReadersOverlap) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::atomic<u32> inside { 0 };
					std::atomic<u32> overlapped { 0 };
					std::atomic<u32> done { 0 };
				};
				auto shared = std::make_shared<Shared>();

				for (u32 t = 0; t < 2; ++t) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								shared->mtx.lock_shared();
								shared->inside.fetch_add(1);
								if (yieldUntil([&] { return shared->inside.load() == 2; }, 2'000)) {
									shared->overlapped.fetch_add(1);
								}
								shared->mtx.unlock_shared();
								shared->done.fetch_add(1);
							}
						)
					);
				}

				ASSERT_TRUE(waitFor([&] { return shared->done.load() == 2; }, 5'000));
				EXPECT_EQ(shared->overlapped.load(), 2u);
			}
		);
	}

	TEST(FiberSharedMutex, WriterExcludesReaders) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::atomic_bool read { false };
				};
				auto shared = std::make_shared<Shared>();

				shared->mtx.lock();
				EXPECT_FALSE(shared->mtx.try_lock_shared());
				EXPECT_FALSE(shared->mtx.try_lock());

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock_shared();
							shared->read = true;
							shared->mtx.unlock_shared();
						}
					)
				);

				EXPECT_FALSE(waitFor([&] { return shared->read.load(); }, 50));
				shared->mtx.unlock();
				EXPECT_TRUE(waitFor([&] { return shared->read.load(); }, 1'000));
				bool locked = false;
				EXPECT_TRUE(waitFor([&] { return locked = locked || shared->mtx.try_lock(); }, 1'000));
				shared->mtx.unlock();
			}
		);
	}

	TEST(FiberSharedMutex, WriterAcquiresUnderSteadyReads) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::chrono::steady_clock::time_point deadline {};
					std::atomic<u32> readersDone { 0 };
					std::atomic<s64> writerMs { -1 };
				};
				auto shared = std::make_shared<Shared>();
				shared->deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);

				for (u32 t = 0; t < 6; ++t) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								while (std::chrono::steady_clock::now() < shared->deadline) {
									shared->mtx.lock_shared();
									::hg::yield();
									shared->mtx.unlock_shared();
								}
								shared->readersDone.fetch_add(1);
							}
						)
					);
				}

				engine::scheduler::thread::self::sleepFor(200);

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							const auto start = std::chrono::steady_clock::now();
							shared->mtx.lock();
							const auto took = std::chrono::steady_clock::now() - start;
							shared->mtx.unlock();
							shared->writerMs = std::chrono::duration_cast<std::chrono::milliseconds>(took).count();
						}
					)
				);

				ASSERT_TRUE(waitFor([&] { return shared->writerMs.load() >= 0; }, 3'000));
				EXPECT_LT(shared->writerMs.load(), 200);
				RecordProperty("writerAcquireMs", std::to_string(shared->writerMs.load()));
				ASSERT_TRUE(waitFor([&] { return shared->readersDone.load() == 6; }, 5'000));
			}
		);
	}

	TEST(FiberSharedMutex, LateReadersWaitBehindWriter) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::atomic<u32> holding { 0 };
					std::atomic_bool release { false };
					std::atomic<u32> seq { 1 };
					std::atomic<u32> writerAt { 0 };
					std::atomic<u32> lateAt { 0 };
				};
				auto shared = std::make_shared<Shared>();

				for (u32 t = 0; t < 2; ++t) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								shared->mtx.lock_shared();
								shared->holding.fetch_add(1);
								yieldUntil([&] { return shared->release.load(); }, 2'000);
								shared->mtx.unlock_shared();
							}
						)
					);
				}
				ASSERT_TRUE(waitFor([&] { return shared->holding.load() == 2; }, 2'000));

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock();
							shared->writerAt = shared->seq.fetch_add(1);
							shared->mtx.unlock();
						}
					)
				);
				ASSERT_TRUE(waitFor([&] { return writerPresent(shared->mtx); }, 2'000));

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock_shared();
							shared->lateAt = shared->seq.fetch_add(1);
							shared->mtx.unlock_shared();
						}
					)
				);
				engine::scheduler::thread::self::sleepFor(20);
				EXPECT_EQ(shared->lateAt.load(), 0u);

				shared->release = true;
				ASSERT_TRUE(waitFor([&] { return shared->lateAt.load() != 0; }, 2'000));
				EXPECT_NE(shared->writerAt.load(), 0u);
				EXPECT_LT(shared->writerAt.load(), shared->lateAt.load());
			}
		);
	}

	TEST(FiberSharedMutex, TwoQueuedWritersBothRun) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::atomic_bool held { false };
					std::atomic_bool released { false };
					std::atomic<u32> acquired { 0 };
					std::atomic<u32> released2 { 0 };
				};
				auto shared = std::make_shared<Shared>();

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock_shared();
							shared->held = true;
							engine::scheduler::thread::self::sleepFor(50);
							shared->mtx.unlock_shared();
							shared->released = true;
						}
					)
				);
				ASSERT_TRUE(waitFor([&] { return shared->held.load(); }, 2'000));

				for (u32 w = 0; w < 2; ++w) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								shared->mtx.lock();
								shared->acquired.fetch_add(1);
								shared->mtx.unlock();
								shared->released2.fetch_add(1);
							}
						)
					);
				}

				ASSERT_TRUE(waitFor([&] { return shared->released.load(); }, 2'000));
				EXPECT_TRUE(waitFor([&] { return shared->released2.load() == 2; }, 1'000));
				EXPECT_EQ(shared->acquired.load(), 2u);
				EXPECT_TRUE(shared->mtx.try_lock());
				shared->mtx.unlock();
			}
		);
	}

	TEST(FiberSharedMutex, ReaderBatchAllHoldBeforeNextWriter) {
		SchedulerModule::structuredRuntimeTest(
			4,
			[](nmpt<engine::Scheduler> s_) {
				struct Shared {
					concurrent::FiberSharedMutex mtx {};
					std::atomic<u32> arrived { 0 };
					std::atomic<u32> holding { 0 };
					std::atomic<u32> sawFour { 0 };
					std::atomic<s32> sawFourAtWriter { -1 };
				};
				auto shared = std::make_shared<Shared>();

				shared->mtx.lock();

				for (u32 t = 0; t < 4; ++t) {
					s_->exec(
						engine::scheduler::task::make_task(
							[shared] {
								shared->arrived.fetch_add(1);
								shared->mtx.lock_shared();
								shared->holding.fetch_add(1);
								if (yieldUntil([&] { return shared->holding.load() == 4; }, 1'000)) {
									shared->sawFour.fetch_add(1);
								}
								shared->mtx.unlock_shared();
							}
						)
					);
				}
				ASSERT_TRUE(waitFor([&] { return shared->arrived.load() == 4; }, 2'000));
				engine::scheduler::thread::self::sleepFor(20);

				s_->exec(
					engine::scheduler::task::make_task(
						[shared] {
							shared->mtx.lock();
							shared->sawFourAtWriter = static_cast<s32>(shared->sawFour.load());
							shared->mtx.unlock();
						}
					)
				);
				engine::scheduler::thread::self::sleepFor(20);
				EXPECT_EQ(shared->holding.load(), 0u);

				shared->mtx.unlock();
				ASSERT_TRUE(waitFor([&] { return shared->sawFourAtWriter.load() >= 0; }, 3'000));
				EXPECT_EQ(shared->sawFourAtWriter.load(), 4);
			}
		);
	}
}
