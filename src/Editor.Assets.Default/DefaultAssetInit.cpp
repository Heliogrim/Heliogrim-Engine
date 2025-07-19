/* Note: Why-so-ever, this is order dependent... */
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets.Type/Accel/AccelEffect.hpp>
#include <Engine.Assets.Type/Audio/Sound.hpp>
#include <Engine.Assets.Type/Geometry/LandscapeGeometry.hpp>
#include <Engine.Assets.Type/Geometry/SkeletalGeometry.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets.Type/Material/PfxMaterial.hpp>
#include <Engine.Assets.Type/Material/SfxMaterial.hpp>
#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
/* Note: Why-so-ever, this is order dependent... */

#include "DefaultAssetInit.hpp"

#include <Engine.Core/Engine.hpp>

#include "Engine.Accel/Accel.hpp"
#include "Engine.Assets.System/AssetDescriptor.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Core/Module/Modules.hpp"
#include "Engine.GFX.Render.Predefined/Effects/BrdfBase.hpp"
#include "Engine.GFX.Render.Predefined/Effects/Skybox.hpp"
#include "Fonts/Default.hpp"
#include "GfxMaterials/Default.hpp"
#include "Images/Default.hpp"
#include "Meshes/Default.hpp"
#include "Textures/Default.hpp"

using namespace hg::editor::boot;
using namespace hg::engine;
using namespace hg;

void editor::boot::initDefaultFontAssets(ref<const engine::Engine> engine_) {

	auto& registry = *engine_.getAssets()->getRegistry();

	/**/

}

void editor::boot::initDefaultGfxMaterialAssets(ref<const engine::Engine> engine_) {

	auto accelSubmodule = engine_.getModules().getSubModule(AccelDepKey);

	auto& registry = *engine_.getAssets()->getRegistry();
	auto& accelGlobalStorage = *static_cast<ptr<engine::Accel>>(accelSubmodule.get())->getGlobalStorage();

	const auto materialDefaultBrdfPath = engine::assets::AssetPath { "materials/default-brdf"sv };
	const auto materialDefaultSkyboxPath = engine::assets::AssetPath { "materials/default-skybox"sv };

	/**/

	const auto brdfBaseEffect = render::makeBrdfBaseEffect();
	const auto brdfEffect = Arci<engine::assets::AccelEffect>::create(
		brdfBaseEffect->getGuid(),
		brdfBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultBrdfPath), "Effect"sv }
	);
	::hg::assertrt(registry.insert({ brdfEffect.into<engine::assets::Asset>() }));

	accelGlobalStorage.insertAccelEffect(accel::calcStorageHash(*brdfBaseEffect), clone(brdfBaseEffect));

	/**/

	auto brdfMaterialPrototype = Arci<engine::assets::GfxMaterialPrototype>::create(
		clone(assets::materials::default_brdf_prototype_guid),
		brdfBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultBrdfPath), "Prototype"sv }
	);
	brdfMaterialPrototype->_effects.emplace_back(brdfBaseEffect->getGuid());
	::hg::assertrt(registry.insert({ brdfMaterialPrototype.into<engine::assets::Asset>() }));

	[](auto& asset_) {
		asset_._params.push_back(
			engine::assets::GfxMaterialPrototype::TmpParam {
				.identifier = engine::gfx::material::ParameterIdentifier { 0u },
				.name = "Albedo",
				.dataType = engine::accel::TransferDataType::eSampler,
				.defaultValue = editor::assets::texture::default_diffuse_guid
			}
		);
		asset_._params.push_back(
			engine::assets::GfxMaterialPrototype::TmpParam {
				.identifier = engine::gfx::material::ParameterIdentifier { 1u },
				.name = "Normal",
				.dataType = engine::accel::TransferDataType::eSampler,
				.defaultValue = editor::assets::texture::default_normal_guid
			}
		);
		asset_._params.push_back(
			engine::assets::GfxMaterialPrototype::TmpParam {
				.identifier = engine::gfx::material::ParameterIdentifier { 2u },
				.name = "Arm",
				.dataType = engine::accel::TransferDataType::eSampler,
				// Warning: This will actually break, because we change from roughness only to arm texture!
				.defaultValue = editor::assets::texture::default_roughness_guid
			}
		);
	}(*brdfMaterialPrototype);

	/**/

	auto brdfMaterial = Arci<engine::assets::GfxMaterial>::create(
		clone(assets::materials::default_brdf_guid),
		brdfBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultBrdfPath), "Material"sv },
		clone(assets::materials::default_brdf_prototype_guid)
	);
	::hg::assertrt(registry.insert({ ::hg::move(brdfMaterial).into<engine::assets::Asset>() }));

	/**/

	const auto skyboxBaseEffect = render::makeSkyboxEffect();
	const auto skyboxEffect = Arci<engine::assets::AccelEffect>::create(
		skyboxBaseEffect->getGuid(),
		skyboxBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultSkyboxPath), "Effect"sv }
	);
	::hg::assertrt(registry.insert({ skyboxEffect.into<engine::assets::Asset>() }));

	accelGlobalStorage.insertAccelEffect(accel::calcStorageHash(*skyboxBaseEffect), clone(skyboxBaseEffect));

	/**/

	auto skyboxMaterialPrototype = Arci<engine::assets::GfxMaterialPrototype>::create(
		clone(assets::materials::default_skybox_prototype_guid),
		skyboxBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultSkyboxPath), "Prototype"sv }
	);
	skyboxMaterialPrototype->_effects.emplace_back(skyboxBaseEffect->getGuid());
	::hg::assertrt(registry.insert({ skyboxMaterialPrototype.into<engine::assets::Asset>() }));

	[](auto& asset_) {
		asset_._params.push_back(
			engine::assets::GfxMaterialPrototype::TmpParam {
				.identifier = engine::gfx::material::ParameterIdentifier { 0u },
				.name = "Texture",
				.dataType = engine::accel::TransferDataType::eSampler,
				.defaultValue = editor::assets::texture::default_skybox_guid
			}
		);
	}(*skyboxMaterialPrototype);

	/**/

	auto skyboxMaterial = Arci<engine::assets::GfxMaterial>::create(
		clone(assets::materials::default_skybox_guid),
		skyboxBaseEffect->getName(),
		engine::assets::AssetReferenceUrl {},
		engine::assets::AssetUrl { clone(materialDefaultSkyboxPath), "Material"sv },
		clone(assets::materials::default_skybox_prototype_guid)
	);
	::hg::assertrt(registry.insert({ ::hg::move(skyboxMaterial).into<engine::assets::Asset>() }));

}

void editor::boot::initDefaultImageAssets(ref<const engine::Engine> engine_) {}

void editor::boot::initDefaultMeshAssets(ref<const engine::Engine> engine_) {}

void editor::boot::initDefaultTextureAssets(ref<const engine::Engine> engine_) {}
