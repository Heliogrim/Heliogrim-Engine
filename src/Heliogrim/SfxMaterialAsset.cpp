#include "SfxMaterialAsset.hpp"

#include <Engine.Assets/Types/SfxMaterial.hpp>

using namespace hg;

SfxMaterialAsset::SfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::SfxMaterial::typeId,
        nullptr
    ) {}

SfxMaterialAsset::~SfxMaterialAsset() noexcept = default;

bool SfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SfxMaterial::typeId;
}
