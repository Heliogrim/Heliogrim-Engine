#include "GeometryAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<LandscapeGeometry>::getStaticTypeId<LandscapeGeometry>() noexcept {
	return LandscapeGeometry::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<SkeletalGeometry>::getStaticTypeId<SkeletalGeometry>() noexcept {
	return SkeletalGeometry::typeId;
}

template <>
template <>
AssetTypeId TypedAssetHandle<StaticGeometry>::getStaticTypeId<StaticGeometry>() noexcept {
	return StaticGeometry::typeId;
}
