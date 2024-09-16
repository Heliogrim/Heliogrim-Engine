#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "PointLightActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<PointLightActor>::serialize(
		const PointLightActor& self_,
		mref<StructScopedSlot> slot_
	) {

		slot_.insertSlot<u64>(Symbols::Type) << PointLightActor::typeId.data;

		/**/

		access::Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot(Symbols::Guid));
		access::Structure<PointLightComponent>::serialize(
			*self_._pointLightComponent,
			slot_.insertStructSlot("point")
		);
	}

	template <>
	void access::Structure<PointLightActor>::hydrate(
		cref<StructScopedSlot> slot_,
		ref<PointLightActor> target_
	) {

		type_id checkTypeId {};
		slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
		::hg::assertrt(checkTypeId == PointLightActor::typeId);

		/**/

		access::Structure<Guid>::hydrate(slot_.getStructSlot(Symbols::Guid), target_._guid);
		access::Structure<PointLightComponent>::hydrate(
			slot_.getStructSlot("point"),
			*target_._pointLightComponent
		);
	}
}
