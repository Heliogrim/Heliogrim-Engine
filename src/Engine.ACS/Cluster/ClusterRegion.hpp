#pragma once

#include <cstddef>
#include <memory>
#include <span>
#include <vector>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/FnRef.hpp>

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
	 */
	class ClusterRegion {
	public:
		static constexpr u32 chunkBytes = 16u * 1024u;

		ClusterRegion(ClusterDesc desc_, ref<HandleTable> handles_);

		ClusterRegion(const ClusterRegion&) = delete;

		ClusterRegion& operator=(const ClusterRegion&) = delete;

		~ClusterRegion();

	public:
		[[nodiscard]] std::byte* allocate(Handle& out_);

		/**
		 * @returns the slots, valid until the next batch allocate
		 */
		std::span<std::byte*> allocate(size_t count_, std::span<Handle> out_);

		void free(std::byte* slot_);

		void forEachChunk(FnRef<void(std::byte* base_, u32 count_, u32 stride_, const u64* liveMask_)> fn_) const;

		[[nodiscard]] ChunkStats stats(u32 chunk_) const;

		/**
		 * Releases every slot and its handle and returns all chunks.
		 */
		void releaseAll();

		/**
		 * Test-only: moves the live slots of one chunk into free slots of other chunks and rebinds their handles.
		 */
		RelocationTable debugMoveChunk(u32 chunk_, FnRef<void(std::byte* src_, std::byte* dst_)> relocate_);

	private:
		struct Chunk {
			std::byte* base;
			u32 live = 0u;
			u32 churn = 0u;
			std::vector<u64> liveMask;
			std::vector<Handle> handles;
		};

		struct Slot {
			Chunk* chunk;
			u32 index;
		};

		[[nodiscard]] Slot claim(const Chunk* exclude_);

		[[nodiscard]] Slot find(const std::byte* slot_) const;

		void returnChunk(const Chunk* chunk_);

		ClusterDesc _desc;
		ref<HandleTable> _handles;
		u32 _chunkSize;
		u32 _capacity;
		std::vector<std::unique_ptr<Chunk>> _chunks;
		std::vector<std::byte*> _batch;
	};
}
