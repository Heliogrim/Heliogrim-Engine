#pragma once

#include "../Asset.hpp"

namespace hg {
	class LevelAsset :
		public Asset {
	public:
		using this_type = LevelAsset;
		using base_type = Asset;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  internal_ The internal engine state of this asset.
		 */
		LevelAsset(mref<asset_guid> guid_, _In_ ref<::hg::engine::assets::Asset> internal_) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~LevelAsset() noexcept;

	public:
		/**
		 * Query if this is valid type
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns True if valid type, false if not.
		 */
		[[nodiscard]] bool isValidType() const noexcept;
	};
}
