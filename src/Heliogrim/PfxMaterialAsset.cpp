#include "PfxMaterialAsset.hpp"

#include <Engine.Assets/Types/PfxMaterial.hpp>

using namespace hg;

PfxMaterialAsset::PfxMaterialAsset(mref<asset_guid> guid_) noexcept :
    Asset(
        std::move(guid_),
        engine::assets::PfxMaterial::typeId,
        nullptr
    ) {}

PfxMaterialAsset::~PfxMaterialAsset() noexcept = default;

bool PfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::PfxMaterial::typeId;
}
