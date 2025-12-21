#include <Engine.Common/Move.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "DirectionalLightComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<DirectionalLightComponent>::serialize(
	const DirectionalLightComponent& self_,
	mref<StructScopedSlot> record_
) {

	Structure<math::fvec3>::serialize(
		self_._luminance,
		record_.insertStructSlot("luminance")
	);
	Structure<math::fvec3>::serialize(
		reinterpret_cast<cref<math::fvec3>>(self_._direction),
		record_.insertStructSlot("direction")
	);

	/**/

	return access::Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<DirectionalLightComponent>::hydrate(
	cref<StructScopedSlot> record_,
	DirectionalLightComponent& target_
) {
	access::Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	Structure<math::fvec3>::hydrate(
		record_.getStructSlot("luminance"),
		target_._luminance
	);
	Structure<math::fvec3>::hydrate(
		record_.getStructSlot("direction"),
		reinterpret_cast<ref<math::fvec3>>(target_._direction)
	);
}