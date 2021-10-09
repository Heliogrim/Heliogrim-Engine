#include "LandscapeMeshAsset.hpp"

#include <Engine.Assets/Types/LandscapeGeometry.hpp>

using namespace ember;

LandscapeMeshAsset::LandscapeMeshAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::LandscapeGeometry::type_id,
        nullptr
    ) {}

LandscapeMeshAsset::~LandscapeMeshAsset() noexcept = default;

bool LandscapeMeshAsset::isValidType() const noexcept {
    return _typeId == engine::assets::LandscapeGeometry::type_id;
}
