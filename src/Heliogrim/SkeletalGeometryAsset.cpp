#include "SkeletalGeometryAsset.hpp"

#include <Engine.Assets/Types/SkeletalGeometry.hpp>

using namespace hg;

SkeletalGeometryAsset::SkeletalGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::SkeletalGeometry::typeId,
        nullptr
    ) {}

SkeletalGeometryAsset::~SkeletalGeometryAsset() noexcept = default;

bool SkeletalGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SkeletalGeometry::typeId;
}
