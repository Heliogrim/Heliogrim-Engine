#pragma once

#include <cstddef>
#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Common/Managed/UniqueRef.hpp>

#include "RelocationTable.hpp"
#include "../Handle/HandleTable.hpp"

namespace hg::engine::acs {
	/**
	 * Slot shape of one archetype: `[component block][actor object]`, the actor at `actorOffset`.
	 */
	struct ClusterDesc {
		u32 stride;
		u32 align;
		u32 actorOffset;
	};

	/**
	 * Counters for later compaction; no policy reads them yet.
	 */
	struct ChunkStats {
		u32 live;
		u32 capacity;
		// free slots below the highest live slot
		u32 holes;
		// allocations plus frees since the chunk was created
		u32 churn;
	};

	/**
	 * Slots of one archetype in chunks of `chunkBytes`; handles bind to the actor of each slot.
	 * The region hands out raw memory only, constructing and destroying slot contents is the caller's job. Not thread-safe.
	 * The HandleTable must outlive the region.
	 */
	class ClusterRegion {
	public:
		constexpr static u32 chunkBytes = 16u * 1024u;

		ClusterRegion(ClusterDesc desc_, ref<HandleTable> handles_);

		ClusterRegion(const ClusterRegion&) = delete;

		ClusterRegion& operator=(const ClusterRegion&) = delete;

		~ClusterRegion();

	public:
		[[nodiscard]] ptr<std::byte> allocate(ref<Handle> out_);

		/**
		 * Panics if `out_` holds fewer than `count_` handles.
		 * @returns the slots, valid until the next batch allocate
		 */
		std::span<ptr<std::byte>> allocate(size_t count_, std::span<Handle> out_);

		void free(ptr<std::byte> slot_);

		/**
		 * Frees a slot and its handle now, but keeps the slot from reuse until `reclaim`.
		 * Raw pointers taken earlier in the frame phase never alias a new occupant.
		 */
		void retire(ptr<std::byte> slot_);

		/**
		 * Makes retired slots claimable again and returns chunks left empty.
		 *
		 * @returns true if any retired slot became claimable
		 */
		bool reclaim();

		/**
		 * Releases every slot and its handle and returns all chunks.
		 */
		void releaseAll();

	public:
		/**
		 * @returns the handle of a live slot, or an invalid handle if the slot is not live in this region
		 */
		[[nodiscard]] Handle handleOf(ptr<const std::byte> slot_) const;

		/** `count_` is the chunk capacity; `liveMask_` marks the live slots. */
		void forEachChunk(FnRef<void(ptr<std::byte> base_, u32 count_, u32 stride_, ptr<const u64> liveMask_)> fn_) const;

		[[nodiscard]] ChunkStats stats(u32 chunk_) const;

		[[nodiscard]] u32 chunkCount() const noexcept;

	public:
		/**
		 * Test-only: moves the live slots of one chunk into free slots of other chunks and rebinds their handles.
		 */
		RelocationTable debugMoveChunk(u32 chunk_, FnRef<void(ptr<std::byte> src_, ptr<std::byte> dst_)> relocate_);

	private:
		struct Chunk {
			ptr<std::byte> base;
			u32 live = 0u;
			u32 churn = 0u;
			u32 retired = 0u;
			Vector<u64> liveMask;
			Vector<u64> retiredMask;
			Vector<Handle> handles;
		};

		struct Slot {
			Chunk* chunk;
			u32 index;
		};

	private:
		ClusterDesc _desc;
		ref<HandleTable> _handles;
		u32 _chunkSize;
		u32 _capacity;
		Vector<UniqueRef<Chunk>> _chunks;
		// chunks sorted by base address, for free()
		Vector<ptr<Chunk>> _byBase;
		Vector<ptr<std::byte>> _batch;

	private:
		[[nodiscard]] Slot claim(ptr<const Chunk> exclude_);

		[[nodiscard]] static uintptr_t chunkAddress(ptr<const Chunk> chunk_) noexcept;

		[[nodiscard]] Slot find(ptr<const std::byte> slot_) const;

		void release(const ptr<const Chunk> chunk_);
	};
}
