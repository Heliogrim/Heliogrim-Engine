#pragma once

#include <compare>
#include <Engine.Common/Types.hpp>

namespace hg::engine::acs {
	// Strong ids: no implicit conversion from integers, so Handle::make arguments cannot be swapped.
	enum class SpaceId : u16 {};

	enum class HandleGeneration : u32 {};

	enum class HandleIndex : u32 {};

	/**
	 * Detached space: handles that belong to no cluster region.
	 */
	inline constexpr SpaceId detachedSpace { 0u };

	/**
	 * Generational handle; bit layout (msb to lsb): 12 bit space | 20 bit generation | 32 bit index.
	 *
	 * shortcut: 20-bit generation wraps after 1M reuses of one index, upgrade to 64-bit handles if long sessions hit it
	 */
	struct Handle {
	public:
		constexpr static u64 indexBits = 32uLL;
		constexpr static u64 generationBits = 20uLL;
		constexpr static u64 spaceBits = 12uLL;

		constexpr static u64 generationMask = (1uLL << generationBits) - 1uLL;
		constexpr static u64 spaceMask = (1uLL << spaceBits) - 1uLL;

	public:
		constexpr Handle() noexcept = default;

	private:
		constexpr explicit Handle(const u64 bits_) noexcept :
			bits(bits_) {}

	public:
		[[nodiscard]] constexpr bool operator==(const Handle&) const noexcept = default;

		[[nodiscard]] constexpr std::strong_ordering operator<=>(const Handle&) const noexcept = default;

	private:
		u64 bits = ~0uLL;

	public:
		[[nodiscard]] constexpr SpaceId space() const noexcept {
			return static_cast<SpaceId>((bits >> (indexBits + generationBits)) & spaceMask);
		}

		[[nodiscard]] constexpr u64 generation() const noexcept {
			return (bits >> indexBits) & generationMask;
		}

		[[nodiscard]] constexpr u32 index() const noexcept {
			return static_cast<u32>(bits & 0xFFFFFFFFuLL);
		}

	public:
		[[nodiscard]] constexpr static Handle make(
			const SpaceId space_,
			const HandleGeneration generation_,
			const HandleIndex index_
		) noexcept {
			return Handle {
				((static_cast<u64>(space_) & spaceMask) << (indexBits + generationBits)) |
				((static_cast<u64>(generation_) & generationMask) << indexBits) |
				static_cast<u64>(index_)
			};
		}
	};
}
