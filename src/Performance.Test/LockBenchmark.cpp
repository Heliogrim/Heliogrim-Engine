#include "pch.h"

/**/
#include <atomic>
#include <chrono>
#include <cstdio>
#include <memory>
#include <Engine.Async/Primitive/FiberMutex.hpp>
#include <Engine.Async/Primitive/FiberSharedMutex.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;

/**
 * Micro benchmarks for the fiber lock primitives.
 *
 * All benchmarks are DISABLED by default and print `ns/op`; run them explicitly with
 *	`--gtest_also_run_disabled_tests --gtest_filter=LockBenchmark.*`
 *
 * Note: Only a debug configuration is built by default, so absolute numbers are indicative; compare before/after only.
 */
namespace Performance {
	namespace {
		constexpr u32 iterations = 200'000;
		constexpr u32 opsPerFiber = 20'000;

		void report(const char* name_, const double nanos_) {
			std::printf("[ BENCH    ] %-48s %10.1f ns/op\n", name_, nanos_);
		}

		[[nodiscard]] u64 nanosSince(const std::chrono::steady_clock::time_point start_) {
			return static_cast<u64>(
				std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_).count()
			);
		}

		template <typename Fn_>
		[[nodiscard]] double nanosPerOp(Fn_&& fn_) {
			for (u32 i = 0; i < iterations / 10; ++i) {
				fn_();
			}
			const auto start = std::chrono::steady_clock::now();
			for (u32 i = 0; i < iterations; ++i) {
				fn_();
			}
			return static_cast<double>(nanosSince(start)) / static_cast<double>(iterations);
		}

		/**
		 * `fibers_` tasks each run `opsPerFiber` lock/unlock pairs; every `writerEvery_`-th op is exclusive, the rest
		 *	shared (all exclusive for a mutex without lock_shared). Reports throughput plus mean and max exclusive-acquire
		 *	latency, the number the fairness of the writer queue shows up in.
		 */
		template <typename Mtx_>
		void contended(const char* name_, const u32 fibers_, const u32 writerEvery_) HG_NO_THREAD_SAFETY_ANALYSIS {
			SchedulerModule::structuredRuntimeTest(
				4,
				[&](nmpt<engine::Scheduler> s_) {
					struct Shared {
						Mtx_ mtx {};
						std::atomic<u32> done { 0 };
						std::atomic<u64> writes { 0 };
						std::atomic<u64> writeNs { 0 };
						std::atomic<u64> writeMaxNs { 0 };
					};
					auto shared = std::make_shared<Shared>();

					const auto start = std::chrono::steady_clock::now();
					for (u32 f = 0; f < fibers_; ++f) {
						s_->exec(
							engine::scheduler::task::make_task(
								[shared, writerEvery_] HG_NO_THREAD_SAFETY_ANALYSIS {
									(void)writerEvery_;	// unused for a mutex without lock_shared
									for (u32 i = 0; i < opsPerFiber; ++i) {
										if constexpr (requires { shared->mtx.lock_shared(); }) {
											if (i % writerEvery_ != 0) {
												shared->mtx.lock_shared();
												shared->mtx.unlock_shared();
												continue;
											}
										}
										const auto t0 = std::chrono::steady_clock::now();
										shared->mtx.lock();
										const auto took = nanosSince(t0);
										shared->mtx.unlock();
										shared->writes.fetch_add(1, std::memory_order::relaxed);
										shared->writeNs.fetch_add(took, std::memory_order::relaxed);
										u64 max = shared->writeMaxNs.load(std::memory_order::relaxed);
										while (took > max && not shared->writeMaxNs.compare_exchange_weak(max, took)) {}
									}
									shared->done.fetch_add(1);
								}
							)
						);
					}

					while (shared->done.load() < fibers_) {
						engine::scheduler::thread::self::sleepFor(1);
					}
					const auto total = nanosSince(start);

					char label[64];
					std::snprintf(label, sizeof(label), "%s f=%u throughput", name_, fibers_);
					report(label, static_cast<double>(total) / static_cast<double>(fibers_ * opsPerFiber));
					std::snprintf(label, sizeof(label), "%s f=%u exclusive acquire mean", name_, fibers_);
					report(label, static_cast<double>(shared->writeNs.load()) / static_cast<double>(shared->writes.load()));
					std::snprintf(label, sizeof(label), "%s f=%u exclusive acquire max", name_, fibers_);
					report(label, static_cast<double>(shared->writeMaxNs.load()));
				}
			);
		}
	}

	TEST(LockBenchmark, DISABLED_SharedMutexUncontended) HG_NO_THREAD_SAFETY_ANALYSIS {
		concurrent::FiberSharedMutex mtx {};
		report("FiberSharedMutex lock/unlock", nanosPerOp([&] { mtx.lock(); mtx.unlock(); }));
		report("FiberSharedMutex lock_shared/unlock_shared", nanosPerOp([&] { mtx.lock_shared(); mtx.unlock_shared(); }));
		concurrent::FiberMutex plain {};
		report("FiberMutex lock/unlock", nanosPerOp([&] { plain.lock(); plain.unlock(); }));
	}

	TEST(LockBenchmark, DISABLED_SharedMutexContended) {
		for (const u32 fibers : { 2u, 4u, 8u }) {
			contended<concurrent::FiberSharedMutex>("FiberSharedMutex 1/8 writes", fibers, 8);
		}
	}

	TEST(LockBenchmark, DISABLED_MutexContended) {
		for (const u32 fibers : { 2u, 4u, 8u }) {
			contended<concurrent::FiberMutex>("FiberMutex", fibers, 1);
		}
	}
}
