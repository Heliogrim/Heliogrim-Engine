#include "pch.h"

/**/

import Engine.Common;
import Engine.Graphics;
import std;

using namespace hg;
using namespace hg::engine::graphics;
using namespace hg::engine::graphics::memory;

namespace GraphicsMemory {
	/* Hands out fake device memory blocks, so the pool runs without a device. */
	class FakeNativeAllocator final :
		public Allocator {
	public:
		[[nodiscard]] AllocationResult allocate(cref<MemoryLayout> layout_, const u64 size_, ref<uptr<AllocatedMemory>> dst_) override {
			const auto handle = _next.fetch_add(1uLL, std::memory_order_relaxed);
			dst_ = make_uptr<AllocatedMemory>(this, nullptr, layout_, size_, 0uLL, nullptr, reinterpret_cast<decltype(AllocatedMemory::vkMemory)>(handle));
			_live.fetch_add(1, std::memory_order_relaxed);
			return AllocationResult::eSuccess;
		}

		void free(mref<uptr<AllocatedMemory>> mem_) override {
			mem_->vkMemory = nullptr;
			mem_.reset();
			_live.fetch_sub(1, std::memory_order_relaxed);
		}

		std::atomic<u64> _next { 0x1000uLL };
		std::atomic<s64> _live { 0 };
	};

	TEST(GlobalPooledAllocatorTest, ConcurrentPooledAllocateAndFree) {
		const auto native = make_sptr<FakeNativeAllocator>();
		{
			GlobalPooledAllocator alloc { native };
			const MemoryLayout layout { 65536uLL, MemoryProperties { MemoryProperty::eDeviceLocal }, 0x1u };

			constexpr auto threads = 8uz;
			constexpr auto rounds = 500uz;
			std::atomic<u64> failures { 0uLL };
			{
				Vector<std::jthread> workers {};
				for (auto t = 0uz; t < threads; ++t) {
					workers.emplace_back(
						[&] {
							Vector<uptr<AllocatedMemory>> held {};
							for (auto r = 0uz; r < rounds; ++r) {
								uptr<AllocatedMemory> mem {};
								if (alloc.allocate(layout, 65536uLL, mem) != AllocationResult::eSuccess || mem == nullptr) {
									failures.fetch_add(1uLL, std::memory_order_relaxed);
									continue;
								}
								held.emplace_back(std::move(mem));
								// Free in bursts, so pool splits and merges interleave across threads.
								if (held.size() >= 4uz) {
									for (auto& entry : held) {
										AllocatedMemory::free(std::move(entry));
									}
									held.clear();
								}
							}
							for (auto& entry : held) {
								AllocatedMemory::free(std::move(entry));
							}
						}
					);
				}
			}

			EXPECT_EQ(failures.load(), 0uLL);
		}
		EXPECT_EQ(native->_live.load(), 0);
	}
}
