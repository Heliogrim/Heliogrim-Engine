#include "pch.h"

/**/

#include <atomic>
#include <thread>
#include <type_traits>
#include <memory>
#include <vector>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Waitable/Signal.hpp>
#include <Engine.Scheduler/Waitable/JoinCounter.hpp>
#include <Engine.Scheduler/Waitable/ParkQueue.hpp>
#include <Heliogrim/Async/Await.hpp>

using namespace hg::engine::scheduler;
using namespace hg;

namespace SchedulerModule {
	TEST(ParkQueue, FifoOrder) {
		ParkQueue queue {};
		ParkNode a, b, c;
		queue.push(a);
		queue.push(b);
		queue.push(c);

		EXPECT_EQ(queue.popOne(), &a);
		EXPECT_EQ(queue.popOne(), &b);
		EXPECT_EQ(queue.popOne(), &c);
		EXPECT_EQ(queue.popOne(), nullptr);
		EXPECT_TRUE(queue.empty());
	}

	TEST(ParkQueue, TakeAllSplices) {
		ParkQueue queue {};
		ParkNode a, b, c;
		queue.push(a);
		queue.push(b);
		queue.push(c);

		auto* node = queue.takeAll();
		EXPECT_EQ(node, &a);
		EXPECT_EQ(node->next, &b);
		EXPECT_EQ(node->next->next, &c);
		EXPECT_EQ(node->next->next->next, nullptr);
		EXPECT_TRUE(queue.empty());
		EXPECT_EQ(queue.takeAll(), nullptr);
	}

	TEST(ParkQueue, LockedPushesWithoutDeadlock) {
		ParkQueue queue {};
		ParkNode a, b;

		const auto count = queue.locked(
			[&](auto locked_) {
				size_t n = 0;
				if (locked_.empty()) {
					locked_.push(a);
					locked_.push(b);
					n = 2;
				}
				return n;
			}
		);

		EXPECT_EQ(count, 2u);
		EXPECT_EQ(queue.popOne(), &a);
		EXPECT_EQ(queue.popOne(), &b);
		EXPECT_EQ(queue.popOne(), nullptr);
	}

	namespace {
		struct CountedNode : ParkNode {
			u32 visited = 0;
		};
	}

	TEST(ParkQueue, ConcurrentPushPop) {
		constexpr size_t threads = 4;
		constexpr size_t perThread = 1000;

		ParkQueue queue {};
		std::vector<CountedNode> nodes(threads * perThread);

		std::vector<std::thread> pushers {};
		for (size_t t = 0; t < threads; ++t) {
			pushers.emplace_back(
				[&, t] {
					for (size_t i = 0; i < perThread; ++i) {
						queue.push(nodes[t * perThread + i]);
					}
				}
			);
		}

		size_t seen = 0;
		while (seen < threads * perThread) {
			if (auto* node = queue.popOne()) {
				static_cast<CountedNode*>(node)->visited++;
				++seen;
			}
		}
		for (auto& th : pushers) {
			th.join();
		}

		for (const auto& node : nodes) {
			EXPECT_EQ(node.visited, 1u);
		}
		EXPECT_TRUE(queue.empty());
	}

	TEST(Signal, WaitAfterSetReturnsInline) {
		engine::scheduler::Signal event {};
		EXPECT_FALSE(event.isSet());
		event.set();
		event.wait();
		EXPECT_TRUE(event.isSet());
	}

	TEST(Signal, ResetRearms) {
		engine::scheduler::Signal event {};
		event.set();
		EXPECT_TRUE(event.isSet());
		event.reset();
		EXPECT_FALSE(event.isSet());
		event.set();
		event.wait();
		EXPECT_TRUE(event.isSet());
	}

	TEST(Signal, ResetThenWaitParks) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				engine::scheduler::Signal event {};
				event.set();
				event.reset();

				std::atomic_flag setOnce {};
				std::atomic_bool timedOut { false };
				std::atomic<bool> setterRan { false };
				std::thread watchdog {
					[&] {
						for (u32 i = 0; i < 2000 && not event.isSet(); ++i) {
							thread::self::sleepFor(1);
						}
						if (not event.isSet()) {
							timedOut = true;
							if (not setOnce.test_and_set()) {
								event.set();
							}
						}
					}
				};
				std::thread setter {
					[&] {
						thread::self::sleepFor(5);
						if (not setOnce.test_and_set()) {
							setterRan = true;
							event.set();
						}
					}
				};

				event.wait();
				EXPECT_TRUE(setterRan.load());
				setter.join();
				watchdog.join();
				ASSERT_FALSE(timedOut.load());
				EXPECT_TRUE(event.isSet());
			}
		);
	}

	TEST(Signal, ThreadWaitsForTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				engine::scheduler::Signal event {};
				std::atomic_flag setOnce {};
				std::atomic_bool timedOut { false };

				// Watchdog keeps the main-thread park from hanging the suite.
				std::thread watchdog {
					[&] {
						for (u32 i = 0; i < 2000 && not event.isSet(); ++i) {
							thread::self::sleepFor(1);
						}
						if (not event.isSet()) {
							timedOut = true;
							if (not setOnce.test_and_set()) {
								event.set();
							}
						}
					}
				};

				s_->exec(
					task::make_task(
						[&] {
							thread::self::sleepFor(5);
							if (not setOnce.test_and_set()) {
								event.set();
							}
						}
					)
				);
				event.wait();
				watchdog.join();
				ASSERT_FALSE(timedOut.load());
				EXPECT_TRUE(event.isSet());
			}
		);
	}

	TEST(Signal, ThousandParkedTasksKeepWorkersFree) {
		structuredRuntimeTest(
			2u,
			[](nmpt<engine::Scheduler> s_) {
				constexpr u32 count = 1000;
				engine::scheduler::Signal event {};
				std::atomic<u32> completed { 0 };
				std::atomic_bool probe { false };

				for (u32 i = 0; i < count; ++i) {
					s_->exec(
						task::make_task(
							[&] {
								event.wait();
								++completed;
							}
						)
					);
				}
				s_->exec(task::make_task([&] { probe = true; }));

				for (u32 i = 0; i < 500 && not probe.load(); ++i) {
					thread::self::sleepFor(1);
				}
				EXPECT_TRUE(probe.load());
				EXPECT_EQ(completed.load(), 0u);

				event.set();
				for (u32 i = 0; i < 5000 && completed.load() != count; ++i) {
					thread::self::sleepFor(1);
				}
				EXPECT_EQ(completed.load(), count);
			}
		);
	}

	TEST(JoinCounter, CompletesAfterAllChildren) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				engine::scheduler::JoinCounter counter { 3 };
				std::atomic<u32> finished { 0 };
				std::atomic_bool timedOut { false };

				// Watchdog keeps the main-thread park from hanging the suite.
				std::thread watchdog {
					[&] {
						for (u32 i = 0; i < 2000 && not counter.isDone(); ++i) {
							thread::self::sleepFor(1);
						}
						if (not counter.isDone()) {
							timedOut = true;
							for (u32 missing = 3 - finished.load(); missing > 0; --missing) {
								counter.done();
							}
						}
					}
				};

				for (u32 i = 0; i < 3; ++i) {
					s_->exec(
						task::make_task(
							[&] {
								++finished;
								counter.done();
							}
						)
					);
				}
				counter.wait();
				watchdog.join();
				ASSERT_FALSE(timedOut.load());
				EXPECT_TRUE(counter.isDone());
			}
		);
	}

	TEST(JoinCounter, GuardPreventsEarlyCompletion) {
		engine::scheduler::JoinCounter counter { 0 };
		counter.add(1);

		std::thread child { [&] { counter.done(); } };
		child.join();
		EXPECT_FALSE(counter.isDone());

		counter.wait();
		EXPECT_TRUE(counter.isDone());
	}

	TEST(JoinCounter, ChildFinishingBeforeNextAddDoesNotComplete) {
		engine::scheduler::JoinCounter counter { 1 };

		std::thread first { [&] { counter.done(); } };
		first.join();
		EXPECT_FALSE(counter.isDone());

		counter.add(1);
		std::thread second { [&] { counter.done(); } };
		second.join();
		EXPECT_FALSE(counter.isDone());

		counter.wait();
		EXPECT_TRUE(counter.isDone());
	}

	TEST(Signal, SetAndDestroyRace) {
		for (u32 i = 0; i < 5000; ++i) {
			auto e = std::make_unique<engine::scheduler::Signal>();
			std::thread setter { [&] { e->set(); } };
			while (not e->isSet()) {}
			e.reset();
			setter.join();
		}
	}

	static_assert(std::is_same_v<hg::Signal, engine::scheduler::Signal>);

	TEST(PublicAwait, EventOverloadParksTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				struct State {
					::hg::Signal event {};
					std::atomic_bool started { false };
					std::atomic_bool done { false };
				};

				auto state = std::make_shared<State>();

				s_->exec(
					task::make_task(
						[state] {
							state->started = true;
							::hg::await(state->event);
							state->done = true;
						}
					)
				);

				for (u32 i = 0; i < 2000 && not state->started.load(); ++i) {
					thread::self::sleepFor(1);
				}
				if (not state->started.load()) {
					state->event.set();
					FAIL() << "task never started";
				}

				// The task cannot pass await before the signal is set.
				const auto prematurelyDone = state->done.load();
				state->event.set();

				for (u32 i = 0; i < 2000 && not state->done.load(); ++i) {
					thread::self::sleepFor(1);
				}
				EXPECT_FALSE(prematurelyDone);
				EXPECT_TRUE(state->done.load());
			}
		);
	}
}
