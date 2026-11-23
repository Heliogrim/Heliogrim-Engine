#pragma once

#include "TypedAssetHandle.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::engine::assets {
	class Sound;
}

namespace hg {
	class SoundAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::Sound> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<::hg::engine::assets::Sound>::getStaticTypeId<::hg::engine::assets::Sound>() noexcept;
}
