#include "MaterialAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<GfxMaterial>::getStaticTypeId<GfxMaterial>() noexcept {
	return GfxMaterial::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<GfxMaterialPrototype>::getStaticTypeId<GfxMaterialPrototype>() noexcept {
	return GfxMaterialPrototype::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<PfxMaterial>::getStaticTypeId<PfxMaterial>() noexcept {
	return PfxMaterial::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<SfxMaterial>::getStaticTypeId<SfxMaterial>() noexcept {
	return SfxMaterial::typeId;
}
