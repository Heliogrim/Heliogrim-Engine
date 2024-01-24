#include "DefaultSkyboxPrototype.hpp"

#include <Engine.Accel/Accel.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Material/GfxMaterialPrototype.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.GFX.Render.Predefined/Effects/Skybox.hpp>

#include "../Textures/DefaultSkybox.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultSkyboxPrototype::DefaultSkyboxPrototype() :
    GfxMatProtoAsset(DefaultSkyboxPrototype::unstable_auto_guid()) {

    /* Warning: Temporary Cross-Fix */

    auto asset = static_cast<ptr<engine::assets::GfxMaterialPrototype>>(internal());
    if (not asset->_effects.empty()) {
        return;
    }

    auto predefined = engine::render::makeSkyboxEffect();

    asset->_effects.emplace_back(predefined->getGuid().data);
    engine::Engine::getEngine()->getAssets()->getFactory()->createAccelEffectAsset(predefined->getGuid().data);

    auto submodule = engine::Engine::getEngine()->getModules().getSubModule(engine::AccelDepKey);
    auto accel = static_cast<ptr<engine::Accel>>(submodule);
    accel->getGlobalStorage()->insertAccelEffect(engine::accel::calcStorageHash(*predefined), std::move(predefined));

    asset->_params.push_back(
        engine::assets::GfxMaterialPrototype::TmpParam {
            .identifier = engine::gfx::material::ParameterIdentifier { 0u },
            .name = "Texture",
            .dataType = engine::accel::TransferDataType::eSampler,
            .defaultValue = asset_guid { texture::DefaultSkybox::unstable_auto_guid() }
        }
    );
}
