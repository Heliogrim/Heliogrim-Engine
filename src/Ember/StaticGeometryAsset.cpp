#include "StaticGeometryAsset.hpp"

#include <Engine.Assets/Types/StaticGeometry.hpp>

using namespace ember;

StaticGeometryAsset::StaticGeometryAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::StaticGeometry::type_id,
        nullptr
    ) {}

StaticGeometryAsset::~StaticGeometryAsset() noexcept = default;

bool StaticGeometryAsset::isValidType() const noexcept {
    return _typeId == engine::assets::StaticGeometry::type_id;
}
