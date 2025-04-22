#pragma once

#include "../Asset.hpp"

namespace hg {
	class LandscapeGeometryAsset :
		public Asset {
	public:
		using this_type = LandscapeGeometryAsset;
		using base_type = Asset;

	protected:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 */
		LandscapeGeometryAsset(mref<AssetGuid> guid_) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~LandscapeGeometryAsset() noexcept;

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
