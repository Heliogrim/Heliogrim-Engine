#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "DirectionalLightComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<DirectionalLightComponent>::serialize(
	const DirectionalLightComponent& self_,
	mref<StructScopedSlot> slot_
) {

	slot_.insertSlot<u64>(Symbols::Type) << DirectionalLightComponent::typeId.data;

	/**/

	Structure<math::fvec3>::serialize(
		self_._luminance,
		slot_.insertStructSlot("luminance")
	);
	Structure<math::fvec3>::serialize(
		reinterpret_cast<cref<math::fvec3>>(self_._direction),
		slot_.insertStructSlot("direction")
	);

}

template <>
void access::Structure<DirectionalLightComponent>::hydrate(
	cref<StructScopedSlot> slot_,
	DirectionalLightComponent& target_
) {
	component_type_id checkTypeId {};
	slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
	::hg::assertrt(checkTypeId == DirectionalLightComponent::typeId);

	/**/

	Structure<math::fvec3>::hydrate(
		slot_.getStructSlot("luminance"),
		target_._luminance
	);
	Structure<math::fvec3>::hydrate(
		slot_.getStructSlot("direction"),
		reinterpret_cast<ref<math::fvec3>>(target_._direction)
	);
}
