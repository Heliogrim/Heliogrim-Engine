#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
/**/
#include "GfxMaterialAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

GfxMaterialAsset::GfxMaterialAsset(
	mref<asset_guid> guid_,
	mref<asset_guid> prototypeGuid_
) noexcept :
	Asset(
		clone(guid_),
		engine::assets::GfxMaterial::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialAsset(
			clone(guid_),
			std::move(prototypeGuid_)
		)
	) {}

GfxMaterialAsset::~GfxMaterialAsset() noexcept = default;

bool GfxMaterialAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::GfxMaterial>(*internal());
}

asset_guid GfxMaterialAsset::getPrototypeGuid() const noexcept {
	return static_cast<cref<::hg::engine::assets::GfxMaterial>>(*internal()).prototype();
}

nmpt<const GfxMatProtoAsset> GfxMaterialAsset::prototype() const noexcept {
	return nullptr;
}
