#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "SpotLightComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<SpotLightComponent>::serialize(
	cref<SpotLightComponent> self_,
	mref<StructScopedSlot> slot_
) {
	slot_.insertSlot<u64>(Symbols::Type) << SpotLightComponent::typeId.data;

	/**/

	slot_.insertSlot<f32>("innerCone") << self_._innerCone;
	slot_.insertSlot<f32>("outerCone") << self_._outerCone;
	slot_.insertSlot<f32>("distanceLimit") << self_._distanceLimit;
	Structure<math::fvec3>::serialize(self_._luminance, slot_.insertStructSlot("luminance"));

}

template <>
void access::Structure<SpotLightComponent>::hydrate(
	cref<StructScopedSlot> slot_,
	SpotLightComponent& target_
) {
	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == SpotLightComponent::typeId);

	/**/

	slot_.getSlot<f32>("innerCone") >> target_._innerCone;
	slot_.getSlot<f32>("outerCone") >> target_._outerCone;
	slot_.getSlot<f32>("distanceLimit") >> target_._distanceLimit;
	Structure<math::fvec3>::hydrate(slot_.getStructSlot("luminance"), target_._luminance);
}
