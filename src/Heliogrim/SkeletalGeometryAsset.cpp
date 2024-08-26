#include "SkeletalGeometryAsset.hpp"

#include <Engine.Assets/Types/SkeletalGeometry.hpp>

using namespace hg;

SkeletalGeometryAsset::SkeletalGeometryAsset(
	mref<asset_guid> guid_,
	ref<::hg::engine::assets::Asset> internal_
) noexcept :
	StreamableRenderableAsset(
		std::move(guid_),
		engine::assets::SkeletalGeometry::typeId,
		internal_
	) {}

SkeletalGeometryAsset::~SkeletalGeometryAsset() noexcept = default;

bool SkeletalGeometryAsset::isValidType() const noexcept {
	return _typeId == engine::assets::SkeletalGeometry::typeId;
}
