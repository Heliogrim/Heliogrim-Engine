#pragma once

#include <Engine.Assets/AssetGuid.hpp>

namespace ember {

	template<typename AssetType_>
	class AssetAutoGuid {
	public:
		/**
		 * Gets the unique identifier
		 *
		 * @author Julius
		 * @date 16.10.2021
		 *
		 * @returns A cref&lt;asset_guid&gt;
		 */
		[[nodiscard]] static cref<asset_guid> auto_guid() noexcept {
			static asset_guid guid = generate_asset_guid();
			return guid;
		}
	};
}
