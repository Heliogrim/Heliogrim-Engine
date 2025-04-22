#pragma once

#include <Engine.Common/String.hpp>

#include "../Asset.hpp"

namespace hg {
	class ImageAsset :
		public Asset {
	public:
		using this_type = ImageAsset;
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
		ImageAsset(mref<AssetGuid> guid_) noexcept;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 27.06.2022
		 *
		 * @param  guid_ Unique identifier.
		 * @param  url_ The url to the underlying data resource
		 */
		ImageAsset(mref<AssetGuid> guid_, cref<string> url_) noexcept;

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~ImageAsset() noexcept;

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

	public:
		[[nodiscard]] cref<string> url() const noexcept;
	};
}
