#include "pch.h"

/**/

#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <vector>
#include <Engine.ACS/Cluster/ClusterRegion.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

using namespace hg::engine::acs;
using namespace hg;

namespace Acs {
	namespace {
		/** Releases the space on scope exit, also after a failed ASSERT. */
		struct SpaceGuard {
			SpaceId id = acquireSpace();

			~SpaceGuard() {
				releaseSpace(id);
			}
		};

		/** Test slot: 32 byte component block followed by a 16 byte actor. */
		constexpr ClusterDesc testDesc { .stride = 48u, .align = 16u, .actorOffset = 32u };

		[[nodiscard]] u32 chunkCount(ref<ClusterRegion> region_) {
			u32 count = 0u;
			region_.forEachChunk([&](std::byte*, u32, u32, const u64*) { ++count; });
			return count;
		}
	}

	TEST(Cluster, AllocateFree) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		ClusterRegion region { testDesc, table };

		Handle handle {};
		auto* const slot = region.allocate(handle);
		ASSERT_NE(slot, nullptr);
		EXPECT_EQ(reinterpret_cast<uintptr_t>(slot) % testDesc.align, 0u);
		EXPECT_EQ(table.resolve(handle), slot + testDesc.actorOffset);
		EXPECT_EQ(region.stats(0u).live, 1u);

		// Pointers outside every chunk are ignored.
		region.free(slot + region.stats(0u).capacity * testDesc.stride);
		EXPECT_EQ(region.stats(0u).live, 1u);

		region.free(slot);
		EXPECT_EQ(table.resolve(handle), nullptr);
		EXPECT_EQ(chunkCount(region), 0u);
	}

	TEST(Cluster, BatchAllocate) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		ClusterRegion region { testDesc, table };

		constexpr size_t n = 10'000uLL;
		Vector<Handle> handles(n);
		const auto slots = region.allocate(n, handles);
		ASSERT_EQ(slots.size(), n);

		std::set<std::byte*> distinct {};
		for (size_t i = 0; i < n; ++i) {
			EXPECT_EQ(reinterpret_cast<uintptr_t>(slots[i]) % testDesc.align, 0u);
			EXPECT_EQ(table.resolve(handles[i]), slots[i] + testDesc.actorOffset);
			distinct.insert(slots[i]);
		}
		EXPECT_EQ(distinct.size(), n);

		// Slots never overlap: sorted neighbours are at least one stride apart.
		for (auto it = distinct.begin(), next = std::next(it); next != distinct.end(); it = next++) {
			EXPECT_GE(*next - *it, static_cast<ptrdiff_t>(testDesc.stride));
		}

		// Dense fill: only the last chunk is partial.
		const auto capacity = region.stats(0u).capacity;
		EXPECT_EQ(chunkCount(region), (n + capacity - 1u) / capacity);
		std::printf(
			"[ cluster  ] stride %u: %u slots per chunk, %u chunks for %zu slots\n",
			testDesc.stride,
			capacity,
			chunkCount(region),
			n
		);
	}

	TEST(ClusterDeathTest, BatchAllocateRejectsShortOutput) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		ClusterRegion region { testDesc, table };

		Vector<Handle> handles(3u);
		EXPECT_DEATH(region.allocate(4u, handles), "");
	}

	TEST(Cluster, RelocationLookupBounds) {
		alignas(16) std::byte from[2u * testDesc.stride] {};
		alignas(16) std::byte to[testDesc.stride] {};
		const RelocationTable relocation { testDesc.stride, { RelocationEntry { from, to } } };

		EXPECT_EQ(relocation.lookup(from), to);
		EXPECT_EQ(relocation.lookup(from + testDesc.stride - 1u), to + testDesc.stride - 1u);
		// One past the slot is not part of it.
		EXPECT_EQ(relocation.lookup(from + testDesc.stride), nullptr);
	}

	TEST(Cluster, ChunkReturnedWhenEmpty) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		ClusterRegion region { testDesc, table };

		Handle first {};
		region.allocate(first);
		const auto capacity = region.stats(0u).capacity;

		Vector<Handle> handles(capacity * 2u);
		const auto batch = region.allocate(handles.size(), handles);
		Vector<std::byte*> slots { batch.begin(), batch.end() };
		ASSERT_EQ(chunkCount(region), 3u);

		// Empty the first chunk only.
		std::byte* firstBase = nullptr;
		region.forEachChunk(
			[&](std::byte* base_, u32, u32, const u64*) {
				if (firstBase == nullptr) {
					firstBase = base_;
				}
			}
		);
		region.free(static_cast<std::byte*>(table.resolve(first)) - testDesc.actorOffset);
		for (auto* const slot : slots) {
			if (slot >= firstBase && slot < firstBase + capacity * testDesc.stride) {
				region.free(slot);
			}
		}
		EXPECT_EQ(chunkCount(region), 2u);

		region.releaseAll();
		EXPECT_EQ(chunkCount(region), 0u);
		EXPECT_EQ(table.resolve(handles.back()), nullptr);
	}

	TEST(Cluster, DebugMoveChunk) {
		const SpaceGuard space {};
		HandleTable table { space.id };
		ClusterRegion region { testDesc, table };

		Handle probe {};
		region.allocate(probe);
		const auto capacity = region.stats(0u).capacity;

		// Fill chunk 0, then put a few slots into chunk 1 so chunk 0's live slots need free room elsewhere.
		Vector<Handle> handles(capacity - 1u + 4u);
		region.allocate(handles.size(), handles);
		handles.emplace_back(probe);

		// Leave holes in chunk 0 so the move handles a sparse live mask.
		std::map<Handle*, std::byte*> before {};
		for (auto& handle : handles) {
			before.emplace(&handle, static_cast<std::byte*>(table.resolve(handle)) - testDesc.actorOffset);
		}
		std::byte* chunk0 = nullptr;
		region.forEachChunk(
			[&](std::byte* base_, u32, u32, const u64*) {
				if (chunk0 == nullptr) {
					chunk0 = base_;
				}
			}
		);
		const auto inChunk0 = [&](const std::byte* slot_) {
			return slot_ >= chunk0 && slot_ < chunk0 + capacity * testDesc.stride;
		};

		u32 dropped = 0u;
		std::set<std::byte*> moved {};
		for (auto it = before.begin(); it != before.end();) {
			if (inChunk0(it->second) && (dropped++ % 3u) == 0u) {
				region.free(it->second);
				it = before.erase(it);
				continue;
			}
			if (inChunk0(it->second)) {
				std::memset(it->second, static_cast<int>(moved.size() & 0x7F), testDesc.stride);
				moved.insert(it->second);
			}
			++it;
		}
		ASSERT_FALSE(moved.empty());

		std::map<std::byte*, std::byte*> copies {};
		const auto relocation = region.debugMoveChunk(
			0u,
			[&](std::byte* src_, std::byte* dst_) {
				std::memcpy(dst_, src_, testDesc.stride);
				copies.emplace(src_, dst_);
			}
		);

		// Every live slot of chunk 0 was relocated once, and the table holds exactly those pairs.
		std::set<std::byte*> copied {};
		for (const auto& [from, to] : copies) {
			copied.insert(from);
		}
		EXPECT_EQ(copied, moved);
		ASSERT_EQ(relocation.entries().size(), moved.size());
		for (const auto& entry : relocation.entries()) {
			EXPECT_EQ(copies.at(entry.from), entry.to);
			EXPECT_FALSE(inChunk0(entry.to));
		}

		for (const auto& [handle, old] : before) {
			auto* const now = static_cast<std::byte*>(table.resolve(*handle)) - testDesc.actorOffset;
			if (moved.contains(old)) {
				EXPECT_EQ(now, copies.at(old));
				EXPECT_EQ(relocation.lookup(old), now);
				// Interior pointer: the actor sub-object maps to the actor in the new slot.
				EXPECT_EQ(relocation.lookup(old + testDesc.actorOffset), now + testDesc.actorOffset);
				EXPECT_EQ(relocation.lookup(old + testDesc.stride - 1u), now + testDesc.stride - 1u);
			} else {
				EXPECT_EQ(now, old);
				EXPECT_EQ(relocation.lookup(old), nullptr);
			}
		}

		// One past the source chunk was never a slot.
		EXPECT_EQ(relocation.lookup(chunk0 + capacity * testDesc.stride), nullptr);

		// The emptied source chunk was returned.
		EXPECT_EQ(chunkCount(region), 1u);
	}
}
