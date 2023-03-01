#include "LandscapeGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/LandscapeGeometry.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

LandscapeGeometryAsset::LandscapeGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::LandscapeGeometry::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createLandscapeGeometryAsset(guid_)
    ) {}

LandscapeGeometryAsset::~LandscapeGeometryAsset() noexcept = default;

bool LandscapeGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::LandscapeGeometry::typeId;
}
