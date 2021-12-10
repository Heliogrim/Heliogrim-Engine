#include "LandscapeGeometryAsset.hpp"

#include <Engine.Assets/Types/LandscapeGeometry.hpp>

using namespace ember;

LandscapeGeometryAsset::LandscapeGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::LandscapeGeometry::type_id,
        nullptr
    ) {}

LandscapeGeometryAsset::~LandscapeGeometryAsset() noexcept = default;

bool LandscapeGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::LandscapeGeometry::type_id;
}
