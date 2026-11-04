#pragma once

#include <cstddef>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "RelocationTable.hpp"

namespace hg::engine::acs {
	class ClusterRegion;

	/**
	 * One chunk of a region to empty into the region's other chunks.
	 */
	struct MoveRequest {
		ptr<ClusterRegion> region;
		u32 chunk;

		void (*relocate)(ptr<std::byte> src_, ptr<std::byte> dst_, u32 count_, u32 stride_);

		u32 stride;
	};

	/**
	 * Collects slot moves during a frame and runs them at the housekeeping point, where no actor pointer is in flight.
	 */
	class RelocationBarrier {
	public:
		void enqueue(MoveRequest request_);

		/**
		 * Drops pending requests of a region about to be freed.
		 */
		void discard(ptr<const ClusterRegion> region_);

		/**
		 * Runs requests per region in descending chunk order; skips (no table) null regions, duplicates and chunk
		 * indices out of range at apply time.
		 *
		 * @returns one table per executed request, in apply order; the caller rebinds external pointers in that order
		 *	and bumps the epoch
		 */
		[[nodiscard]] Vector<RelocationTable> apply();

	private:
		Vector<MoveRequest> _pending;
	};
}
