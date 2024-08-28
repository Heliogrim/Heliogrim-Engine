#pragma once

#include "../Asset.hpp"

namespace hg {
	class StaticGeometryAsset :
		public Asset {
	public:
		using this_type = StaticGeometryAsset;
		using base_type = Asset;

	public:
		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @param  guid_ Unique identifier.
		 */
		StaticGeometryAsset(mref<asset_guid> guid_) noexcept;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 27.06.2022
		 *
		 * @param  guid_ Unique identifier.
		 * @param  url_ The url to the underlying data resource
		 * @param  vertexCount_ The number of vertices within the source geometry
		 * @param  indexCount_ The number of indices within the source geometry
		 */
		StaticGeometryAsset(
			mref<asset_guid> guid_,
			cref<string> url_,
			cref<u64> vertexCount_,
			cref<u64> indexCount_
		) noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~StaticGeometryAsset() noexcept;

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
		[[nodiscard]] u32 getMaterialCount() const;
	};
}
