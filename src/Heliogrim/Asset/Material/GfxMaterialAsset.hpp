#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../Asset.hpp"

namespace hg {
	class GfxMatProtoAsset;

	class GfxMaterialAsset :
		public Asset {
	public:
		using this_type = GfxMaterialAsset;
		using base_type = Asset;

	public:
		GfxMaterialAsset(
			mref<asset_guid> guid_,
			mref<asset_guid> prototypeGuid_
		) noexcept;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 06.10.2021
		 */
		~GfxMaterialAsset() noexcept;

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
		[[nodiscard]] asset_guid getPrototypeGuid() const noexcept;

		[[nodiscard]] nmpt<const GfxMatProtoAsset> prototype() const noexcept;
	};
}
