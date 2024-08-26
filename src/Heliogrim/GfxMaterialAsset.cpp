#include "GfxMaterialAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Material/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

GfxMaterialAsset::GfxMaterialAsset(
	mref<asset_guid> guid_,
	mref<asset_guid> prototypeGuid_
) noexcept :
	StreamableRenderableAsset(
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
	return _typeId == engine::assets::GfxMaterial::typeId;
}

const ptr<const GfxMatProtoAsset> GfxMaterialAsset::prototype() const noexcept {
	return nullptr;
}
