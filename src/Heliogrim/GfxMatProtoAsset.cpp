#include "GfxMatProtoAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Material/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

GfxMatProtoAsset::GfxMatProtoAsset(
    cref<asset_guid> guid_
) noexcept :
    Asset(
        guid_,
        engine::assets::GfxMaterial::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialPrototypeAsset(
            guid_
        )
    ) {}

GfxMatProtoAsset::~GfxMatProtoAsset() noexcept = default;

bool GfxMatProtoAsset::isValidType() const noexcept {
    return _typeId == engine::assets::GfxMaterial::typeId;
}
