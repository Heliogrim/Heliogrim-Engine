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
	[[nodiscard]] LevelGuid generate_level_guid();
}
