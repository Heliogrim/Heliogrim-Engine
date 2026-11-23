#pragma once

#include <Engine.Assets.Type/Universe/LevelAsset.hpp>

#include "TypedAssetHandle.hpp"

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
