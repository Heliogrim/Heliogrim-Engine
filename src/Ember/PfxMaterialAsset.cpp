#include "PfxMaterialAsset.hpp"

#include <Engine.Assets/Types/PfxMaterial.hpp>

using namespace ember;

PfxMaterialAsset::PfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::PfxMaterial::typeId,
        nullptr
    ) {}

PfxMaterialAsset::~PfxMaterialAsset() noexcept = default;

bool PfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::PfxMaterial::typeId;
}
