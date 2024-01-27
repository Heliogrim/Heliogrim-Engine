#include "SfxMaterialAsset.hpp"

#include <Engine.Assets/Types/SfxMaterial.hpp>

using namespace hg;

SfxMaterialAsset::SfxMaterialAsset(mref<asset_guid> guid_) noexcept :
    Asset(
        std::move(guid_),
        engine::assets::SfxMaterial::typeId,
        nullptr
    ) {}

SfxMaterialAsset::~SfxMaterialAsset() noexcept = default;

bool SfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::SfxMaterial::typeId;
}
