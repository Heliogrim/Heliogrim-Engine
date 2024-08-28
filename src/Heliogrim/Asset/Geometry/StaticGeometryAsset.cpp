#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
/**/
#include "StaticGeometryAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

StaticGeometryAsset::StaticGeometryAsset(mref<asset_guid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::StaticGeometry::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createStaticGeometryAsset(clone(guid_))
	) {}

StaticGeometryAsset::StaticGeometryAsset(
	mref<asset_guid> guid_,
	cref<string> url_,
	cref<u64> vertexCount_,
	cref<u64> indexCount_
) noexcept :
	Asset(
		clone(guid_),
		engine::assets::StaticGeometry::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createStaticGeometryAsset(
			clone(guid_),
			url_,
			vertexCount_,
			indexCount_
		)
	) {}

StaticGeometryAsset::~StaticGeometryAsset() noexcept = default;

bool StaticGeometryAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::StaticGeometry>(*internal());
}

u32 StaticGeometryAsset::getMaterialCount() const {

	if (not internal()) {
		return 0uL;
	}

	return static_cast<ptr<engine::assets::StaticGeometry>>(internal())->getMaterialCount();
}
