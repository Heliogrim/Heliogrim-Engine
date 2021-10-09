#include "SkeletalMeshAsset.hpp"

#include <Engine.Assets/Types/SkeletalGeometry.hpp>

using namespace ember;

SkeletalMeshAsset::SkeletalMeshAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::SkeletalGeometry::type_id,
        nullptr
    ) {}

SkeletalMeshAsset::~SkeletalMeshAsset() noexcept = default;

bool SkeletalMeshAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SkeletalGeometry::type_id;
}
