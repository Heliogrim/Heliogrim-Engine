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
	mref<StructScopedSlot> slot_
) {

	slot_.insertSlot<u64>(Symbols::Type) << SkyboxComponent::typeId.data;

	/**/

	// TODO: Implement Resolver
	Structure<Guid>::serialize(self_._skyboxGeometry.guid(), slot_.insertStructSlot("geometry"sv));
	Structure<Guid>::serialize(self_._skyboxMaterial.guid(), slot_.insertStructSlot("material"sv));
}

template <>
void access::Structure<SkyboxComponent>::hydrate(
	cref<StructScopedSlot> slot_,
	SkyboxComponent& target_
) {

	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == SkyboxComponent::typeId);

	/**/

	// TODO: Implement Resolver
	asset_guid geometryGuid {};
	Structure<Guid>::hydrate(slot_.getStructSlot("geometry"sv), geometryGuid);
	target_._skyboxGeometry = GetAssets().find<StaticGeometryAsset>(geometryGuid).value;

	asset_guid materialGuid {};
	Structure<Guid>::hydrate(slot_.getStructSlot("material"sv), materialGuid);
	target_._skyboxMaterial = GetAssets().find<GfxMaterialAsset>(materialGuid).value;
}
