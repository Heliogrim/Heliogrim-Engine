#include "StaticGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

StaticGeometryAsset::StaticGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::StaticGeometry::typeId,
        engine::Session::get()->modules().assetFactory()->createStaticGeometryAsset(guid_)
    ) {}

StaticGeometryAsset::~StaticGeometryAsset() noexcept = default;

bool StaticGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::StaticGeometry::typeId;
}
