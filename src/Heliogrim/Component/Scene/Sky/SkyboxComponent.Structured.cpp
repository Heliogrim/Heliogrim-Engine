#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "SkyboxComponent.hpp"
#include "../../../AssetDatabase.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<SkyboxComponent>::serialize(
	const SkyboxComponent& self_,
	mref<StructScopedSlot> record_
) {

	// TODO: Implement Resolver
	Structure<Guid>::serialize(self_._skyboxGeometry.getAssetGuid(), record_.insertStructSlot("geometry"sv));
	Structure<Guid>::serialize(self_._skyboxMaterial.getAssetGuid(), record_.insertStructSlot("material"sv));
	// Problem: At this point we are losing bytes of the last value stored into a slot
	// Question: Maybe the slots are getting destroyed out of order?

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<SkyboxComponent>::hydrate(
	cref<StructScopedSlot> record_,
	SkyboxComponent& target_
) {
	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	// TODO: Implement Resolver
	AssetGuid geometryGuid {};
	Structure<Guid>::hydrate(record_.getStructSlot("geometry"sv), geometryGuid);
	target_._skyboxGeometry = GetAssets().find<StaticGeometryAssetHandle>(geometryGuid).value;

	AssetGuid materialGuid {};
	Structure<Guid>::hydrate(record_.getStructSlot("material"sv), materialGuid);
	target_._skyboxMaterial = GetAssets().find<GfxMaterialAssetHandle>(materialGuid).value;
}
