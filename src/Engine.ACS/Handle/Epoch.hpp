#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>

namespace hg::engine::acs {
	/**
	 * Global structural epoch; bumped whenever addresses behind handles may have moved.
	 */
	class Epoch {
	public:
		[[nodiscard]] static u64 current() noexcept {
			return counter().load(std::memory_order_acquire);
		}

		static void bump() noexcept {
			counter().fetch_add(1uLL, std::memory_order_acq_rel);
		}

	private:
		static std::atomic_uint64_t& counter() noexcept {
			static std::atomic_uint64_t value { 0uLL };
			return value;
		}
	};
}
