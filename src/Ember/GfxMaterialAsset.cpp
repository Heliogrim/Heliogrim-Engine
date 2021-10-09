#include "GfxMaterialAsset.hpp"

#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.Assets/AssetFactory.hpp>

using namespace ember;

GfxMaterialAsset::GfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::GfxMaterial::type_id,
        engine::assets::AssetFactory::get()->createGfxMaterialAsset(guid_)
    ) {}

GfxMaterialAsset::GfxMaterialAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> albedo_,
    cref<asset_guid> ao_,
    cref<asset_guid> cavity_,
    cref<asset_guid> displacement_,
    cref<asset_guid> gloss_,
    cref<asset_guid> normal_,
    cref<asset_guid> roughness_,
    cref<asset_guid> specular_
) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::GfxMaterial::type_id,
        engine::assets::AssetFactory::get()->createGfxMaterialAsset(
            guid_,
            albedo_,
            ao_,
            cavity_,
            displacement_,
            gloss_,
            normal_,
            roughness_,
            specular_
        )
    ) {}

GfxMaterialAsset::~GfxMaterialAsset() noexcept = default;

bool GfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::GfxMaterial::type_id;
}
