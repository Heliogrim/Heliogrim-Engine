#pragma once

#include "../Asset.hpp"

namespace hg {
	class GfxMatProtoAsset :
		public Asset {
	public:
		using this_type = GfxMatProtoAsset;
		using base_type = Asset;

	public:
		GfxMatProtoAsset(mref<asset_guid> guid_) noexcept;

		~GfxMatProtoAsset() noexcept;

	public:
		[[nodiscard]] bool isValidType() const noexcept;
	};
}
