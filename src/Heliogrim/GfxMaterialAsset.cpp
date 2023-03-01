#include "GfxMaterialAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

GfxMaterialAsset::GfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::GfxMaterial::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialAsset(guid_)
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
    cref<asset_guid> metalness_,
    cref<asset_guid> specular_,
    cref<asset_guid> alpha_
) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::GfxMaterial::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialAsset(
            guid_,
            albedo_,
            ao_,
            cavity_,
            displacement_,
            gloss_,
            normal_,
            roughness_,
            metalness_,
            specular_,
            alpha_
        )
    ) {}

GfxMaterialAsset::~GfxMaterialAsset() noexcept = default;

bool GfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::GfxMaterial::typeId;
}
