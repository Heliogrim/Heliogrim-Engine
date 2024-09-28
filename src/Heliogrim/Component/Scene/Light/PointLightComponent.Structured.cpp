#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "PointLightComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<PointLightComponent>::serialize(
	const PointLightComponent& self_,
	mref<StructScopedSlot> slot_
) {

	slot_.insertSlot<u64>(Symbols::Type) << PointLightComponent::typeId.data;

	/**/

	slot_.insertSlot<f32>("distanceLimit") << self_._distanceLimit;
	Structure<math::fvec3>::serialize(self_._luminance, slot_.insertStructSlot("luminance"));

}

template <>
void access::Structure<PointLightComponent>::hydrate(
	cref<StructScopedSlot> slot_,
	PointLightComponent& target_
) {

	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == PointLightComponent::typeId);

	/**/

	slot_.getSlot<f32>("distanceLimit") >> target_._distanceLimit;
	Structure<math::fvec3>::hydrate(slot_.getStructSlot("luminance"), target_._luminance);
}
