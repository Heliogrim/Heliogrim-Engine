#pragma once

#include <Engine.Assets.Type/Audio/Sound.hpp>

#include "TypedAssetHandle.hpp"

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
