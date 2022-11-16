#include "StaticGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

StaticGeometryAsset::StaticGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::StaticGeometry::typeId,
        engine::Session::get()->modules().assetFactory()->createStaticGeometryAsset(guid_)
    ) {}

StaticGeometryAsset::StaticGeometryAsset(
    cref<asset_guid> guid_,
    cref<string> url_,
    cref<u64> vertexCount_,
    cref<u64> indexCount_
) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::StaticGeometry::typeId,
        engine::Session::get()->modules().assetFactory()->createStaticGeometryAsset(
            guid_,
            url_,
            vertexCount_,
            indexCount_
        )
    ) {}

StaticGeometryAsset::~StaticGeometryAsset() noexcept = default;

bool StaticGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::StaticGeometry::typeId;
}

u32 StaticGeometryAsset::getMaterialCount() const {

    if (not internal()) {
        return 0ui32;
    }

    return static_cast<ptr<engine::assets::StaticGeometry>>(internal())->getMaterialCount();
}
