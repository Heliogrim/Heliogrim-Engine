#include "SfxMaterialAsset.hpp"

#include <Engine.Assets/Types/SfxMaterial.hpp>

using namespace ember;

SfxMaterialAsset::SfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::SfxMaterial::type_id,
        nullptr
    ) {}

SfxMaterialAsset::~SfxMaterialAsset() noexcept = default;

bool SfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SfxMaterial::type_id;
}
