#include "DefaultSkyboxPrototype.hpp"

#include <Engine.Accel/Accel.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Accel/AccelEffect.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.GFX.Render.Predefined/Effects/Skybox.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Default.hpp"
#include "../Textures/Default.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultSkyboxPrototype::DefaultSkyboxPrototype() :
	GfxMatProtoAsset(clone(editor::assets::materials::default_skybox_prototype_guid)) {

	/* Warning: Temporary Cross-Fix */

	auto asset = static_cast<ptr<engine::assets::GfxMaterialPrototype>>(internal());
	if (not asset->_effects.empty()) {
		return;
	}

	auto predefined = engine::render::makeSkyboxEffect();

	asset->_effects.emplace_back(predefined->getGuid());
	engine::Engine::getEngine()->getAssets()->getFactory()->createAccelEffectAsset(predefined->getGuid());

	auto submodule = engine::Engine::getEngine()->getModules().getSubModule(engine::AccelDepKey);
	auto accel = static_cast<ptr<engine::Accel>>(submodule.get());
	accel->getGlobalStorage()->insertAccelEffect(engine::accel::calcStorageHash(*predefined), std::move(predefined));

	asset->_params.push_back(
		engine::assets::GfxMaterialPrototype::TmpParam {
			.identifier = engine::gfx::material::ParameterIdentifier { 0u },
			.name = "Texture",
			.dataType = engine::accel::TransferDataType::eSampler,
			.defaultValue = editor::assets::texture::default_skybox_guid
		}
	);
}
