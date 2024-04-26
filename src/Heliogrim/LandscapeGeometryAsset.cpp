#include "LandscapeGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/LandscapeGeometry.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

LandscapeGeometryAsset::LandscapeGeometryAsset(mref<asset_guid> guid_) noexcept :
	StreamableRenderableAsset(
		clone(guid_),
		engine::assets::LandscapeGeometry::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		engine::Engine::getEngine()->getAssets()->getFactory()->createLandscapeGeometryAsset(clone(guid_)).get()
	) {}

LandscapeGeometryAsset::~LandscapeGeometryAsset() noexcept = default;

bool LandscapeGeometryAsset::isValidType() const noexcept {
	return _typeId == engine::assets::LandscapeGeometry::typeId;
}
