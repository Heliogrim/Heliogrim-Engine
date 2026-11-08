#include "pch.h"

/**/

#include <atomic>
#include <thread>
#include <vector>
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
}
