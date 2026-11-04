#pragma once

#include <cstddef>
#include <vector>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "RelocationTable.hpp"

namespace hg::engine::acs {
	class ClusterRegion;

	/**
	 * One chunk of a region to empty into the region's other chunks.
	 */
	struct MoveRequest {
		ptr<ClusterRegion> region;
		u32 chunk;
		void (*relocate)(std::byte* src_, std::byte* dst_, u32 count_, u32 stride_);
		u32 stride;
	};

	/**
	 * Collects slot moves during a frame and runs them at the housekeeping point, where no actor pointer is in flight.
	 */
	class RelocationBarrier {
	public:
		void enqueue(MoveRequest request_);

		/**
		 * @returns one table per request, in enqueue order; the caller rebinds external pointers and bumps the epoch
		 */
		[[nodiscard]] std::vector<RelocationTable> apply();

	private:
		std::vector<MoveRequest> _pending;
	};
}
