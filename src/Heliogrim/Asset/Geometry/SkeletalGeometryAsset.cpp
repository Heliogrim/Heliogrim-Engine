#include <Engine.Assets/Types/SkeletalGeometry.hpp>
/**/
#include "SkeletalGeometryAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

SkeletalGeometryAsset::SkeletalGeometryAsset(
	mref<asset_guid> guid_,
	ref<::hg::engine::assets::Asset> internal_
) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::SkeletalGeometry::typeId,
		internal_
	) {}

SkeletalGeometryAsset::~SkeletalGeometryAsset() noexcept = default;

bool SkeletalGeometryAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::SkeletalGeometry>(*internal());
}
