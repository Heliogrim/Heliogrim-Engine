#include "LandscapeGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/LandscapeGeometry.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

LandscapeGeometryAsset::LandscapeGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::LandscapeGeometry::type_id,
        engine::Session::get()->modules().assetFactory()->createLandscapeGeometryAsset(guid_)
    ) {}

LandscapeGeometryAsset::~LandscapeGeometryAsset() noexcept = default;

bool LandscapeGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::LandscapeGeometry::type_id;
}
