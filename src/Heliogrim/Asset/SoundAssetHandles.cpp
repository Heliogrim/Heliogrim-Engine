#include "SoundAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<Sound>::getStaticTypeId<Sound>() noexcept {
	return Sound::typeId;
}
