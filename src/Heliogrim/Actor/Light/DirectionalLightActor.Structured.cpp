#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "DirectionalLightActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<DirectionalLightActor>::serialize(
		const DirectionalLightActor& self_,
		mref<StructScopedSlot> slot_
	) {

		slot_.insertSlot<u64>(Symbols::Type) << DirectionalLightActor::typeId.data;

		/**/

		access::Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot(Symbols::Guid));
		access::Structure<DirectionalLightComponent>::serialize(
			*self_._directionalLightComponent,
			slot_.insertStructSlot("directional")
		);
	}

	template <>
	void access::Structure<DirectionalLightActor>::hydrate(
		cref<StructScopedSlot> slot_,
		ref<DirectionalLightActor> target_
	) {

		type_id checkTypeId {};
		slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
		::hg::assertrt(checkTypeId == DirectionalLightActor::typeId);

		/**/

		access::Structure<Guid>::hydrate(slot_.getStructSlot(Symbols::Guid), target_._guid);
		access::Structure<DirectionalLightComponent>::hydrate(
			slot_.getStructSlot("directional"),
			*target_._directionalLightComponent
		);
	}
}
