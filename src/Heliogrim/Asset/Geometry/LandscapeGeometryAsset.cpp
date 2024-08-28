#include <Engine.Assets/Types/LandscapeGeometry.hpp>
/**/
#include "LandscapeGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

LandscapeGeometryAsset::LandscapeGeometryAsset(mref<asset_guid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::LandscapeGeometry::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createLandscapeGeometryAsset(clone(guid_))
	) {}

LandscapeGeometryAsset::~LandscapeGeometryAsset() noexcept = default;

bool LandscapeGeometryAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::LandscapeGeometry>(*internal());
}
