#include "StaticMeshAsset.hpp"

#include <Engine.Assets/Types/StaticGeometry.hpp>

using namespace ember;

StaticMeshAsset::StaticMeshAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::StaticGeometry::type_id,
        nullptr
    ) {}

StaticMeshAsset::~StaticMeshAsset() noexcept = default;

bool StaticMeshAsset::isValidType() const noexcept {
    return _typeId == engine::assets::StaticGeometry::type_id;
}
