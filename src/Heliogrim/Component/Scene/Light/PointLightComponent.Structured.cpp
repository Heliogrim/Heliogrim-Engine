#include <Engine.Common/Move.hpp>
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
	mref<StructScopedSlot> record_
) {
	record_.insertSlot<f32>("distanceLimit") << self_._distanceLimit;
	Structure<math::fvec3>::serialize(self_._luminance, record_.insertStructSlot("luminance"));

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<PointLightComponent>::hydrate(
	cref<StructScopedSlot> record_,
	PointLightComponent& target_
) {
	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	record_.getSlot<f32>("distanceLimit") >> target_._distanceLimit;
	Structure<math::fvec3>::hydrate(record_.getStructSlot("luminance"), target_._luminance);
}