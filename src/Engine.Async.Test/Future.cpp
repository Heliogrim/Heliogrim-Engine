#include "pch.h"

/**/

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <Engine.Async/Future/Async.hpp>
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

	namespace {
		template <typename T_, typename E_>
		[[nodiscard]] bool waitReady(const Future<T_, E_>& future_) {
			for (u32 i = 0; i < 2000 && not future_.isReady(); ++i) {
				engine::scheduler::thread::self::sleepFor(1);
			}
			return future_.isReady();
		}
	}

	TEST(Future, ThenBeforeResolveRunsOnce) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> promise {};
				std::atomic<int> count { 0 };
				std::atomic<std::thread::id> where {};

				auto chained = promise.future().then(
					[&](int v_) {
						++count;
						where = std::this_thread::get_id();
						return v_ + 1;
					}
				);
				promise.resolve(1);

				ASSERT_TRUE(waitReady(chained));
				auto result = await(std::move(chained));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 2);
				EXPECT_EQ(count.load(), 1);
				EXPECT_NE(where.load(), std::this_thread::get_id());
			}
		);
	}

	TEST(Future, ThenAfterResolveRunsOnce) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> promise {};
				auto future = promise.future();
				promise.resolve(1);

				std::atomic<int> count { 0 };
				std::atomic<std::thread::id> where {};
				auto chained = std::move(future).then(
					[&](int v_) {
						++count;
						where = std::this_thread::get_id();
						return v_ + 1;
					}
				);

				ASSERT_TRUE(waitReady(chained));
				auto result = await(std::move(chained));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 2);
				EXPECT_EQ(count.load(), 1);
				EXPECT_NE(where.load(), std::this_thread::get_id());
			}
		);
	}

	TEST(Future, ThenAfterResolveThenDropRunsOnce) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				std::atomic<int> count { 0 };
				std::optional<Future<int>> chained {}; {
					Promise<int> promise {};
					auto future = promise.future();
					promise.resolve(1);
					chained.emplace(
						std::move(future).then(
							[&](int v_) {
								++count;
								return v_;
							}
						)
					);
				}

				ASSERT_TRUE(waitReady(*chained));
				auto result = await(std::move(*chained));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 1);
				EXPECT_EQ(count.load(), 1);
			}
		);
	}

	TEST(Future, ThenOnReadyFutureRuns) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				auto chained = Future<int>::ready(1).then([](int v_) { return v_ + 1; });
				ASSERT_TRUE(waitReady(chained));
				auto result = await(std::move(chained));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 2);
			}
		);
	}

	TEST(Future, ThenNeverOnResolverStack) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> promise {};
				std::atomic_bool gate { false };
				std::atomic_bool ran { false };
				bool ranBeforeReturn = true;

				auto chained = promise.future().then(
					[&](int v_) {
						// Inline execution would hold the resolver here until the bound expires, setting the flag first.
						for (u32 i = 0; i < 2000 && not gate.load(); ++i) {
							engine::scheduler::thread::self::sleepFor(1);
						}
						ran = true;
						return v_;
					}
				);

				std::thread resolver {
					[&] {
						promise.resolve(3);
						ranBeforeReturn = ran.load();
						gate = true;
					}
				};
				resolver.join();
				EXPECT_FALSE(ranBeforeReturn);

				ASSERT_TRUE(waitReady(chained));
				auto result = await(std::move(chained));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 3);
				EXPECT_TRUE(ran.load());
			}
		);
	}

	TEST(Future, ChainOfTenThousandLinks) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> promise {};
				auto future = promise.future();
				for (u32 i = 0; i < 10'000; ++i) {
					future = std::move(future).then([](int v_) { return v_ + 1; });
				}
				promise.resolve(0);

				ASSERT_TRUE(waitReady(future));
				auto result = await(std::move(future));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 10'000);
			}
		);
	}

	TEST(Future, BrokenChainOfTenThousandLinks) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				std::atomic<int> count { 0 };
				std::optional<Promise<int>> promise { std::in_place };
				auto future = promise->future();
				for (u32 i = 0; i < 10'000; ++i) {
					future = std::move(future).then(
						[&](int v_) {
							++count;
							return v_ + 1;
						}
					);
				}
				promise.reset();

				ASSERT_TRUE(waitReady(future));
				auto result = await(std::move(future));
				ASSERT_FALSE(result.has_value());
				EXPECT_EQ(result.error().reason, Broken::Reason::eDropped);
				EXPECT_EQ(count.load(), 0);
			}
		);
	}

	TEST(Future, ResultContinuationMapsError) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> promise {};
				auto mapped = promise.future().then(
					[](int) -> Result<int, std::string> { return Unexpected<std::string> { "bad" }; }
				);
				promise.resolve(1);

				ASSERT_TRUE(waitReady(mapped));
				auto result = await(std::move(mapped));
				ASSERT_FALSE(result.has_value());
				ASSERT_TRUE(result.error().is<std::string>());
				EXPECT_EQ(result.error().as<std::string>(), "bad");

				// An upstream error is converted without calling the continuation.
				std::atomic<int> count { 0 };
				Promise<int, int> typed {};
				auto converted = typed.future().then(
					[&](int v_) -> Result<int, long long> {
						++count;
						return v_;
					}
				);
				typed.reject(3);

				ASSERT_TRUE(waitReady(converted));
				auto convertedResult = await(std::move(converted));
				ASSERT_FALSE(convertedResult.has_value());
				ASSERT_TRUE(convertedResult.error().is<long long>());
				EXPECT_EQ(convertedResult.error().as<long long>(), 3);
				EXPECT_EQ(count.load(), 0);
			}
		);
	}

	TEST(Future, BrokenSkipsContinuation) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				std::atomic<int> count { 0 };
				std::optional<Promise<int>> promise { std::in_place };
				auto chained = promise->future().then(
					[&](int v_) {
						++count;
						return v_;
					}
				);
				promise.reset();

				ASSERT_TRUE(waitReady(chained));
				auto result = await(std::move(chained));
				ASSERT_FALSE(result.has_value());
				EXPECT_EQ(result.error().reason, Broken::Reason::eDropped);
				EXPECT_EQ(count.load(), 0);
			}
		);
	}

	TEST(Future, FlattenInnerFuture) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> outer {};
				Promise<int> inner {};
				std::optional<Future<int>> innerFuture { inner.future() };

				Future<int> flat = outer.future().then([&](int) { return std::move(*innerFuture); });
				outer.resolve(1);
				engine::scheduler::thread::self::sleepFor(5);
				EXPECT_FALSE(flat.isReady());
				inner.resolve(5);

				ASSERT_TRUE(waitReady(flat));
				auto result = await(std::move(flat));
				ASSERT_TRUE(result.has_value());
				EXPECT_EQ(result.value(), 5);
			}
		);
	}

	TEST(Future, FlattenInnerDroppedIsBroken) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				Promise<int> outer {};
				std::optional<Promise<int>> inner { std::in_place };
				std::optional<Future<int>> innerFuture { inner->future() };

				Future<int> flat = outer.future().then([&](int) { return std::move(*innerFuture); });
				outer.resolve(1);
				inner.reset();

				ASSERT_TRUE(waitReady(flat));
				auto result = await(std::move(flat));
				ASSERT_FALSE(result.has_value());
				EXPECT_EQ(result.error().reason, Broken::Reason::eDropped);
			}
		);
	}

	TEST(Future, AsyncOneLiner) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				auto f = async([] { return 7; });
				ASSERT_TRUE(waitReady(f));
				EXPECT_EQ(await(std::move(f)).value(), 7);
			}
		);
	}

	TEST(Future, AsyncResultRejects) {
		SchedulerModule::structuredRuntimeTest(
			[](nmpt<engine::Scheduler>) {
				auto f = async([]() -> Result<int, std::string> { return Unexpected<std::string> { "no" }; });
				ASSERT_TRUE(waitReady(f));
				auto result = await(std::move(f));
				ASSERT_FALSE(result.has_value());
				ASSERT_TRUE(result.error().is<std::string>());
				EXPECT_EQ(result.error().as<std::string>(), "no");
			}
		);
	}
}
