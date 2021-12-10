#include "SkeletalGeometryAsset.hpp"

#include <Engine.Assets/Types/SkeletalGeometry.hpp>

using namespace ember;

SkeletalGeometryAsset::SkeletalGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::SkeletalGeometry::type_id,
        nullptr
    ) {}

SkeletalGeometryAsset::~SkeletalGeometryAsset() noexcept = default;

bool SkeletalGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SkeletalGeometry::type_id;
}
