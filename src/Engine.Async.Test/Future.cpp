#include "pch.h"

/**/

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <Engine.Async/Future/Future.hpp>
#include <Engine.Async/Future/Promise.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "../Engine.Scheduler.Test/TestSchedulerEngine.hpp"

using namespace hg;

namespace AsyncModule {
	TEST(Future, ResolveThenAwait) {
		Promise<int> promise {};
		auto future = promise.future();
		EXPECT_FALSE(future.isReady());
		EXPECT_TRUE(promise.isPending());

		promise.resolve(42);
		EXPECT_FALSE(promise.isPending());
		EXPECT_TRUE(future.isReady());

		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 42);
	}

	TEST(Future, AwaitThenResolveFromThread) {
		Promise<int> promise {};
		auto future = promise.future();

		std::thread resolver {
			[&] {
				engine::scheduler::thread::self::sleepFor(5);
				promise.resolve(7);
			}
		};

		auto result = await(std::move(future));
		resolver.join();
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 7);
	}

	TEST(Future, AwaitFromFiberParks) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				Promise<int> promise {};
				auto future = promise.future();
				std::atomic_bool started { false };
				std::atomic_bool finished { false };
				std::atomic<int> value { 0 };

				s_->exec(
					engine::scheduler::task::make_task(
						[&] {
							started = true;
							auto result = await(std::move(future));
							value = result.has_value() ? result.value() : -1;
							finished = true;
						}
					)
				);

				for (u32 i = 0; i < 2000 && not started.load(); ++i) {
					engine::scheduler::thread::self::sleepFor(1);
				}
				engine::scheduler::thread::self::sleepFor(5);
				EXPECT_FALSE(finished.load());
				promise.resolve(11);

				for (u32 i = 0; i < 2000 && not finished.load(); ++i) {
					engine::scheduler::thread::self::sleepFor(1);
				}
				ASSERT_TRUE(started.load());
				ASSERT_TRUE(finished.load());
				EXPECT_EQ(value.load(), 11);
			}
		);
	}

	TEST(Future, DroppedPromiseIsBroken) {
		std::optional<Future<int>> future {}; {
			Promise<int> promise {};
			future.emplace(promise.future());
		}
		ASSERT_TRUE(future->isReady());

		auto result = await(std::move(*future));
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().reason, Broken::Reason::eDropped);
	}

	TEST(Future, RejectDefaultIsBrokenRejected) {
		Promise<int> promise {};
		auto future = promise.future();
		promise.reject(Broken { Broken::Reason::eRejected });

		auto result = await(std::move(future));
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().reason, Broken::Reason::eRejected);
	}

	TEST(Future, RejectTypedError) {
		Promise<int, std::string> promise {};
		auto future = promise.future();
		promise.reject(std::string { "bad" });

		auto result = await(std::move(future));
		ASSERT_FALSE(result.has_value());
		ASSERT_TRUE(result.error().is<std::string>());
		EXPECT_EQ(result.error().as<std::string>(), "bad");
	}

	TEST(Future, VoidResolve) {
		Promise<void> promise {};
		auto future = promise.future();
		promise.resolve();

		auto result = await(std::move(future));
		EXPECT_TRUE(result.has_value());
	}

	TEST(Future, VoidRejectTyped) {
		Promise<void, int> promise {};
		auto future = promise.future();
		promise.reject(3);

		auto result = await(std::move(future));
		ASSERT_FALSE(result.has_value());
		ASSERT_TRUE(result.error().is<int>());
		EXPECT_EQ(result.error().as<int>(), 3);
	}

	TEST(Future, ReadyAwaitsInline) {
		auto future = Future<int>::ready(5);
		EXPECT_TRUE(future.isReady());
		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		EXPECT_EQ(result.value(), 5);

		auto none = Future<void>::ready();
		EXPECT_TRUE(none.isReady());
		EXPECT_TRUE(await(std::move(none)).has_value());
	}

	TEST(Future, ValueMovedOnce) {
		Promise<uptr<int>> promise {};
		auto future = promise.future();
		promise.resolve(std::make_unique<int>(9));

		auto result = await(std::move(future));
		ASSERT_TRUE(result.has_value());
		ASSERT_NE(result.value(), nullptr);
		EXPECT_EQ(*result.value(), 9);
		// The future is consumed by await.
		EXPECT_FALSE(future.isReady());
	}

	namespace {
		struct Counted {
			static inline u32 destroyed = 0;
			bool owner = true;

			Counted() = default;

			Counted(Counted&& other_) noexcept :
				owner(std::exchange(other_.owner, false)) {}

			~Counted() {
				if (owner) {
					++destroyed;
				}
			}
		};
	}

	TEST(Future, StateFreedAfterBothHandles) {
		Counted::destroyed = 0;

		std::optional<Future<Counted>> future {}; {
			Promise<Counted> promise {};
			future.emplace(promise.future());
			promise.resolve(Counted {});
		}
		EXPECT_EQ(Counted::destroyed, 0u);
		future.reset();
		EXPECT_EQ(Counted::destroyed, 1u); {
			Promise<Counted> promise {}; {
				auto dropped = promise.future();
			}
			promise.resolve(Counted {});
			EXPECT_EQ(Counted::destroyed, 2u);
		}
		EXPECT_EQ(Counted::destroyed, 2u);
	}
}
