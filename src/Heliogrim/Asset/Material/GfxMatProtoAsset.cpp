#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
/**/
#include "GfxMatProtoAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

GfxMatProtoAsset::GfxMatProtoAsset(
	mref<AssetGuid> guid_
) noexcept :
	Asset(
		clone(guid_),
		engine::assets::GfxMaterialPrototype::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createGfxMaterialPrototypeAsset(clone(guid_))
	) {}

GfxMatProtoAsset::~GfxMatProtoAsset() noexcept = default;

bool GfxMatProtoAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::GfxMaterialPrototype>(*internal());
}
