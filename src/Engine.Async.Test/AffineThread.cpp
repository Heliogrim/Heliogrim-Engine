#include "pch.h"

/**/

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <Engine.Async/Affine/AffineThread.hpp>
#include <Engine.Async/Future/Future.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

using namespace hg;
using namespace hg::engine::async;

namespace AsyncModule {
	namespace {
		constexpr auto watchdog = std::chrono::seconds { 5 };

		[[nodiscard]] bool waitUntil(auto&& pred_) {
			const auto deadline = std::chrono::steady_clock::now() + watchdog;
			while (not pred_()) {
				if (std::chrono::steady_clock::now() >= deadline) {
					return false;
				}

				std::this_thread::yield();
			}
			return true;
		}

		template <typename T_>
		[[nodiscard]] bool waitReady(ref<const Future<T_>> future_) {
			return waitUntil([&] { return future_.isReady(); });
		}

		struct Harness {
			std::counting_semaphore<> wakes { 0 };
			// A late waker leaves the constructor waker empty; the loop installs it itself, like a platform thread.
			const bool lateWaker;
			std::function<void()> beforeWaker;
			std::atomic_bool exited { false };
			AffineThread thread;

			// Rejection, depth and throw paths log; scoped per test like the scheduler fixture, so later suites can set up again.
			explicit Harness(const bool lateWaker_ = false) :
				lateWaker(lateWaker_),
				thread("Test-Affine", lateWaker_ ? AffineWaker {} : waker()) {
				Logger::setup();
			}

			~Harness() {
				thread.stop();
				Logger::destroy();
			}

			[[nodiscard]] AffineWaker waker() {
				return [this] { wakes.release(); };
			}

			void start(std::function<void()> beforeWaker_ = {}) {
				// Kept as a member: a std::function capture would exceed the UniqueFn loop buffer.
				beforeWaker = std::move(beforeWaker_);
				thread.start(
					[this](ref<AffineThread> self_) {
						if (beforeWaker) {
							beforeWaker();
						}
						if (lateWaker) {
							self_.setWaker(waker());
						}
						loop(self_);
						exited = true;
					}
				);
			}

			// The documented loop contract, verbatim.
			void loop(ref<AffineThread> self_) {
				for (;;) {
					self_.drain();
					if (self_.stopping()) {
						self_.drain();
						return;
					}
					if (not wakes.try_acquire_for(watchdog)) {
						ADD_FAILURE() << "affine loop watchdog";
						return;
					}
				}
			}
		};

		// Blocks the loop until opened; bounded so a broken test cannot hang the run.
		struct Gate {
			std::atomic_bool entered { false };
			std::atomic_bool open { false };

			void block() {
				entered = true;
				(void)waitUntil([this] { return open.load(); });
			}
		};
	}

	TEST(AffineThread, PostRunsOnThread) {
		Harness h {};
		h.start();

		auto future = h.thread.post(
			[] {
				return AffineThread::current() != nullptr && engine::scheduler::thread::self::isAffine();
			}
		);
		ASSERT_TRUE(waitReady(future));
		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		EXPECT_TRUE(result.value());
		EXPECT_EQ(AffineThread::current(), nullptr);
	}

	TEST(AffineThread, PerProducerFifo) {
		constexpr u32 producers = 8u;
		constexpr u32 items = 1000u;

		Harness h {};
		std::vector<std::pair<u32, u32>> seen {};
		h.start();

		std::vector<std::thread> threads {};
		for (u32 p = 0; p < producers; ++p) {
			threads.emplace_back(
				[&h, &seen, p] {
					for (u32 i = 0; i < items; ++i) {
						h.thread.postDetached([&seen, p, i] { seen.emplace_back(p, i); });
					}
				}
			);
		}
		for (auto& thread : threads) {
			thread.join();
		}
		h.thread.stop();

		ASSERT_EQ(seen.size(), producers * items);
		std::vector<u32> next(producers, 0u);
		for (const auto& [p, i] : seen) {
			EXPECT_EQ(i, next[p]);
			next[p] = i + 1u;
		}
	}

	TEST(AffineThread, StopRunsQueuedThenRejects) {
		Harness h {};
		Gate gate {};
		std::atomic<u32> ran { 0u };
		h.start();

		h.thread.postDetached([&gate] { gate.block(); });
		ASSERT_TRUE(waitUntil([&] { return gate.entered.load(); }));
		for (u32 i = 0; i < 100u; ++i) {
			h.thread.postDetached([&ran] { ++ran; });
		}

		std::thread stopper { [&h] { h.thread.stop(); } };
		ASSERT_TRUE(waitUntil([&] { return h.thread.stopping(); }));
		gate.open = true;
		stopper.join();

		EXPECT_EQ(ran.load(), 100u);

		auto rejected = h.thread.post([] { return 1; });
		ASSERT_TRUE(rejected.isReady());
		auto result = await(std::move(rejected));
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().reason, Broken::Reason::eRejected);

		auto shared = std::make_shared<int>(0);
		h.thread.postDetached([shared] { ++*shared; });
		EXPECT_EQ(shared.use_count(), 1);
		EXPECT_EQ(*shared, 0);
	}

	TEST(AffineThread, PostBeforeStartRuns) {
		Harness h {};
		auto future = h.thread.post([] { return 5; });
		EXPECT_FALSE(future.isReady());

		h.start();
		ASSERT_TRUE(waitReady(future));
		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 5);
	}

	TEST(AffineThread, ThrowingItemDropsAndContinues) {
		Harness h {};
		h.start();

		auto failing = h.thread.post([]() -> int { throw std::runtime_error("affine item failure"); });
		auto following = h.thread.post([] { return 3; });

		ASSERT_TRUE(waitReady(failing));
		ASSERT_TRUE(waitReady(following));

		auto failed = await(std::move(failing));
		ASSERT_FALSE(failed.has_value());
		EXPECT_EQ(failed.error().reason, Broken::Reason::eDropped);

		auto result = await(std::move(following));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 3);
	}

	TEST(AffineThread, NestedDrainKeepsFifo) {
		Harness h {};
		Gate gate {};
		std::vector<std::string> order {};
		std::atomic_bool finished { false };
		h.start();

		h.thread.postDetached([&gate] { gate.block(); });
		h.thread.postDetached(
			[&order, &finished] {
				order.emplace_back("A-start");
				auto* const self = AffineThread::current();
				self->postDetached([&order] { order.emplace_back("B"); });
				self->postDetached([&order] { order.emplace_back("C"); });
				// Simulates a modal loop re-entering the drain.
				self->drain();
				order.emplace_back("A-end");
				finished = true;
			}
		);
		h.thread.postDetached([&order] { order.emplace_back("X"); });
		gate.open = true;

		ASSERT_TRUE(waitUntil([&] { return finished.load(); }));
		// Joining orders the affine writes before the reads below.
		h.thread.stop();

		const std::vector<std::string> expected { "A-start", "X", "B", "C", "A-end" };
		EXPECT_EQ(order, expected);
	}

	TEST(AffineThread, LateWakerPostBeforeStartRuns) {
		Harness h { true };
		auto future = h.thread.post([] { return 7; });
		EXPECT_FALSE(future.isReady());

		h.start();
		ASSERT_TRUE(waitReady(future));
		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 7);
	}

	TEST(AffineThread, StopBeforeWakerSetExits) {
		Harness h { true };
		Gate gate {};
		std::atomic_bool stopped { false };
		h.start([&gate] { gate.block(); });
		ASSERT_TRUE(waitUntil([&] { return gate.entered.load(); }));

		// The stop wake hits the still-empty waker; only the contract's stopping() check lets the loop exit.
		std::thread stopper {
			[&h, &stopped] {
				h.thread.stop();
				stopped = true;
			}
		};
		ASSERT_TRUE(waitUntil([&] { return h.thread.stopping(); }));
		gate.open = true;

		EXPECT_TRUE(waitUntil([&] { return stopped.load(); }));
		stopper.join();
	}

	TEST(AffineThread, ConcurrentStopWaitsForJoin) {
		Harness h {};
		Gate gate {};
		h.start();
		h.thread.postDetached([&gate] { gate.block(); });
		ASSERT_TRUE(waitUntil([&] { return gate.entered.load(); }));

		std::atomic_bool firstSawExit { false };
		std::atomic_bool secondSawExit { false };
		std::thread first {
			[&] {
				h.thread.stop();
				firstSawExit = h.exited.load();
			}
		};
		std::thread second {
			[&] {
				h.thread.stop();
				secondSawExit = h.exited.load();
			}
		};
		ASSERT_TRUE(waitUntil([&] { return h.thread.stopping(); }));
		// Keep the loop busy so the losing stop() overlaps the winner's join.
		std::this_thread::sleep_for(std::chrono::milliseconds { 50 });
		gate.open = true;
		first.join();
		second.join();

		EXPECT_TRUE(firstSawExit.load());
		EXPECT_TRUE(secondSawExit.load());
	}

	#ifdef _DEBUG
	TEST(AffineThread, DepthWarningOncePerCrossing) {
		Harness h {};
		h.start();

		for (u64 round = 1uLL; round <= 2uLL; ++round) {
			Gate gate {};
			h.thread.postDetached([&gate] { gate.block(); });
			for (u32 i = 0; i < 1100u; ++i) {
				h.thread.postDetached([] {});
			}
			EXPECT_GT(h.thread.depth(), 1024uLL);
			EXPECT_EQ(h.thread.warningsIssued(), round);

			gate.open = true;
			ASSERT_TRUE(waitUntil([&] { return h.thread.depth() == 0uLL; }));
		}
	}

	TEST(AffineThread, StopFromItselfAsserts) {
		EXPECT_DEATH(
			{
				Harness h {};
				h.start();
				auto future = h.thread.post([] { AffineThread::current()->stop(); });
				(void)waitReady(future);
			},
			""
		);
	}
	#endif
}
