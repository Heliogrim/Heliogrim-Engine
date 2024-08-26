#pragma once

#include "Asset.hpp"
#include "AssetAutoGuid.hpp"

namespace hg {
	class StreamableAsset :
		public Asset {
	public:
		using base_type = Asset;

		using value_type = StreamableAsset;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 * @param  typeId_ Identifier for the type.
		 * @param  internal_ The internal state of this asset.
		 */
		StreamableAsset(
			mref<asset_guid> guid_,
			cref<asset_type_id> typeId_,
			_In_ ref<::hg::engine::assets::Asset> internal_
		) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~StreamableAsset() noexcept;
	};
}
