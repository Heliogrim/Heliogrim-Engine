#pragma once

#include "../Asset.hpp"

namespace hg {
	class SoundAsset :
		public Asset {
	public:
		using this_type = SoundAsset;
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
		SoundAsset(mref<asset_guid> guid_, _In_ ref<::hg::engine::assets::Asset> internal_) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~SoundAsset() noexcept;

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
