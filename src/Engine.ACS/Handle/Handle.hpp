#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::acs {
	using SpaceId = u16;

	/**
	 * Detached space: handles that belong to no cluster region.
	 */
	constexpr SpaceId detachedSpace = 0u;

	/**
	 * Generational handle; bit layout (msb to lsb): 12 bit space | 20 bit generation | 32 bit index.
	 *
	 * shortcut: 20-bit generation wraps after 1M reuses of one index, upgrade to 64-bit handles if long sessions hit it
	 */
	struct Handle {
		static constexpr u64 indexBits = 32uLL;
		static constexpr u64 generationBits = 20uLL;
		static constexpr u64 spaceBits = 12uLL;

		static constexpr u64 generationMask = (1uLL << generationBits) - 1uLL;
		static constexpr u64 spaceMask = (1uLL << spaceBits) - 1uLL;

		u64 bits = ~0uLL;

		[[nodiscard]] static constexpr Handle make(const SpaceId space_, const u64 generation_, const u32 index_) noexcept {
			return Handle {
				((static_cast<u64>(space_) & spaceMask) << (indexBits + generationBits)) |
				((generation_ & generationMask) << indexBits) |
				static_cast<u64>(index_)
			};
		}

		[[nodiscard]] constexpr SpaceId space() const noexcept {
			return static_cast<SpaceId>((bits >> (indexBits + generationBits)) & spaceMask);
		}

		[[nodiscard]] constexpr u64 generation() const noexcept {
			return (bits >> indexBits) & generationMask;
		}

		[[nodiscard]] constexpr u32 index() const noexcept {
			return static_cast<u32>(bits & 0xFFFFFFFFuLL);
		}

		[[nodiscard]] constexpr bool operator==(const Handle&) const noexcept = default;
	};
}
