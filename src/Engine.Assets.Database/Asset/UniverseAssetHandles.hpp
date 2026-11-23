#pragma once

#include "TypedAssetHandle.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::engine::assets {
	class LevelAsset;
}

namespace hg {
	class LevelAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::LevelAsset> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<::hg::engine::assets::LevelAsset>::getStaticTypeId<::hg::engine::assets::LevelAsset>() noexcept;
}
