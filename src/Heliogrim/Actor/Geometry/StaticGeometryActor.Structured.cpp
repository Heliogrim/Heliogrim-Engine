#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "StaticGeometryActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<StaticGeometryActor>::serialize(
		const StaticGeometryActor& self_,
		mref<StructScopedSlot> slot_
	) {

		slot_.insertSlot<u64>(Symbols::Type) << StaticGeometryActor::typeId.data;

		/**/

		access::Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot(Symbols::Guid));
		access::Structure<StaticGeometryComponent>::serialize(
			*self_._staticGeometryComponent,
			slot_.insertStructSlot("geometry")
		);
	}

	template <>
	void access::Structure<StaticGeometryActor>::hydrate(
		cref<StructScopedSlot> slot_,
		ref<StaticGeometryActor> target_
	) {

		type_id checkTypeId {};
		slot_.getSlot<u64>(Symbols::Type) >> checkTypeId.data;
		::hg::assertrt(checkTypeId == StaticGeometryActor::typeId);

		/**/

		access::Structure<Guid>::hydrate(slot_.getStructSlot(Symbols::Guid), target_._guid);
		access::Structure<StaticGeometryComponent>::hydrate(
			slot_.getStructSlot("geometry"),
			*target_._staticGeometryComponent
		);
	}
}
