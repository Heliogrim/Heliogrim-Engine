#include "pch.h"

/**/

#include <atomic>
#include <thread>
#include <vector>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Waitable/Event.hpp>
#include <Engine.Scheduler/Waitable/ParkQueue.hpp>

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

	TEST(Event, WaitAfterSetReturnsInline) {
		engine::scheduler::Event event {};
		EXPECT_FALSE(event.isSet());
		event.set();
		event.wait();
		EXPECT_TRUE(event.isSet());
	}

	TEST(Event, ThreadWaitsForTask) {
		structuredRuntimeTest(
			[](nmpt<engine::Scheduler> s_) {
				engine::scheduler::Event event {};
				s_->exec(
					task::make_task(
						[&] {
							thread::self::sleepFor(5);
							event.set();
						}
					)
				);
				event.wait();
				EXPECT_TRUE(event.isSet());
			}
		);
	}

	TEST(Event, ThousandParkedTasksKeepWorkersFree) {
		structuredRuntimeTest(
			2u,
			[](nmpt<engine::Scheduler> s_) {
				constexpr u32 count = 1000;
				engine::scheduler::Event event {};
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
}
