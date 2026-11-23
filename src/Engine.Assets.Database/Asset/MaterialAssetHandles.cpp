#include "MaterialAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<GraphicsMaterial>::getStaticTypeId<GraphicsMaterial>() noexcept {
	return GraphicsMaterial::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<GraphicsMaterialPrototype>::getStaticTypeId<GraphicsMaterialPrototype>() noexcept {
	return GraphicsMaterialPrototype::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<PhysicsMaterial>::getStaticTypeId<PhysicsMaterial>() noexcept {
	return PhysicsMaterial::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<AudioMaterial>::getStaticTypeId<AudioMaterial>() noexcept {
	return AudioMaterial::typeId;
}
