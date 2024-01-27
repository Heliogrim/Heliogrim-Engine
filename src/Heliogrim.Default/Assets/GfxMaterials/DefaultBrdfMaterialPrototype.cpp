#include "DefaultBrdfMaterialPrototype.hpp"

#include <Engine.Accel/Accel.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Material/GfxMaterialPrototype.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.GFX.Render.Predefined/Effects/BrdfBase.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Heliogrim.Default/Assets/Textures/DefaultDiffuse.hpp"
#include "Heliogrim.Default/Assets/Textures/DefaultNormal.hpp"
#include "Heliogrim.Default/Assets/Textures/DefaultRoughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultBrdfMaterialPrototype::DefaultBrdfMaterialPrototype() :
    GfxMatProtoAsset(clone(DefaultBrdfMaterialPrototype::unstable_auto_guid())) {

    /* Warning: Temporary Cross-Fix */

    auto asset = static_cast<ptr<engine::assets::GfxMaterialPrototype>>(internal());
    if (not asset->_effects.empty()) {
        return;
    }

    auto predefined = engine::render::makeBrdfBaseEffect();

    asset->_effects.emplace_back(predefined->getGuid().data);
    engine::Engine::getEngine()->getAssets()->getFactory()->createAccelEffectAsset(predefined->getGuid().data);

    auto submodule = engine::Engine::getEngine()->getModules().getSubModule(engine::AccelDepKey);
    auto accel = static_cast<ptr<engine::Accel>>(submodule);
    accel->getGlobalStorage()->insertAccelEffect(engine::accel::calcStorageHash(*predefined), std::move(predefined));

    asset->_params.push_back(
        engine::assets::GfxMaterialPrototype::TmpParam {
            .identifier = engine::gfx::material::ParameterIdentifier { 0u },
            .name = "Albedo",
            .dataType = engine::accel::TransferDataType::eSampler,
            //.defaultValue = asset_guid { texture::DefaultDiffuse::unstable_auto_guid() }
            .defaultValue = asset_guid { decodeGuid4228("1513411454-43702-17570-414804866245091486"sv).data }
        }
    );
    asset->_params.push_back(
        engine::assets::GfxMaterialPrototype::TmpParam {
            .identifier = engine::gfx::material::ParameterIdentifier { 1u },
            .name = "Normal",
            .dataType = engine::accel::TransferDataType::eSampler,
            //.defaultValue = asset_guid { texture::DefaultNormal::unstable_auto_guid() }
            .defaultValue = asset_guid { decodeGuid4228("2957839326-51539-17441-445097573227334828"sv).data }
        }
    );
    asset->_params.push_back(
        engine::assets::GfxMaterialPrototype::TmpParam {
            .identifier = engine::gfx::material::ParameterIdentifier { 2u },
            .name = "Arm",
            .dataType = engine::accel::TransferDataType::eSampler,
            // Warning: This will actually break, because we change from roughness only to arm texture!
            //.defaultValue = asset_guid { texture::DefaultRoughness::unstable_auto_guid() }
            .defaultValue = asset_guid { decodeGuid4228("3532173099-19939-16567-8852593991083949459"sv).data }
        }
    );
}
