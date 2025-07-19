#include "TextureAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<FontAsset>::getStaticTypeId<FontAsset>() noexcept {
	return FontAsset::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<ImageAsset>::getStaticTypeId<ImageAsset>() noexcept {
	return ImageAsset::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<TextureAsset>::getStaticTypeId<TextureAsset>() noexcept {
	return TextureAsset::typeId;
}
