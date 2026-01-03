#pragma once

#include <Engine.Common/Guid.hpp>

namespace hg {
	struct LevelGuid :
		public Guid {
		using Guid::Guid;

		constexpr explicit LevelGuid(ref<const Guid> guid_) noexcept :
			Guid(guid_) {}
	};

	/**
	 * Generates an level unique identifier
	 * @returns The level unique identifier.
	 */
	[[nodiscard]] inline LevelGuid generate_level_guid() {
		static std::atomic_uint_fast32_t ai { 1 };
		const auto pre = ai.fetch_add(1, std::memory_order_relaxed);
		const auto post = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		return { pre, u16 { 2u }, u16 { 2u }, static_cast<u64>(post) };
	}
}
