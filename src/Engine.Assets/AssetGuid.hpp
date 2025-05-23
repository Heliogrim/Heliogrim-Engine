#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Hash/Murmur3.hpp>

namespace hg {
	struct asset_guid : public Guid {
		using Guid::Guid;

		asset_guid(cref<Guid> other_) noexcept :
			Guid(other_) {}
	};

	/**
	 * Identifier for the invalid asset
	 */
	inline constexpr static asset_guid invalid_asset_guid {};

	/**
	 * Generates an unstable asset unique identifier
	 *
	 * @author Julius
	 * @date 23.10.2020
	 *
	 * @returns The asset unique identifier.
	 */
	extern asset_guid generate_unstable_asset_guid();

	/**
	 * Generates an asset unique identifier
	 *
	 * @author Julius
	 * @date 02.03.2020
	 *
	 * @returns The asset unique identifier.
	 */
	extern asset_guid generate_asset_guid();
}

namespace std {
	template <>
	struct hash<hg::asset_guid> {
		std::size_t operator()(const hg::asset_guid& value_) const noexcept {

			constexpr hg::u64 size = sizeof(hg::asset_guid);

			/**
			 * Warning: Don't use identity
			 */
			hg::u64 dst[2];
			hg::hash::murmur3_x64_128(
				&value_,
				static_cast<hg::u32>(size),
				0x9FB21C651E98DF25uLL,
				&dst
			);
			return dst[0];
		}
	};
}
