#include "GfxMaterialAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Material/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

GfxMaterialAsset::GfxMaterialAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::GfxMaterial::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialAsset(guid_)
    ) {}

GfxMaterialAsset::~GfxMaterialAsset() noexcept = default;

bool GfxMaterialAsset::isValidType() const noexcept {
    return _typeId == engine::assets::GfxMaterial::typeId;
}
