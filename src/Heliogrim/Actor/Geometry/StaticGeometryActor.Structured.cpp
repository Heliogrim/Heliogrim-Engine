#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "StaticGeometryActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<StaticGeometryActor>::serialize(
		const StaticGeometryActor& self_,
		mref<StructScopedSlot> record_
	) {

		insert_named_guid_slot(record_, "geometry"sv, self_._staticGeometryComponent->guid());

		/**/

		access::Structure<Actor>::serialize(self_, ::hg::move(record_));
	}

	template <>
	void access::Structure<StaticGeometryActor>::hydrate(
		cref<StructScopedSlot> record_,
		ref<StaticGeometryActor> target_
	) {

		access::Structure<Actor>::hydrate(record_, target_);

		/**/

		const auto staticGeometryComponentGuid = get_named_guid_slot(record_, "geometry"sv);
		target_._staticGeometryComponent = nullptr;

		for (auto* const candidate : target_._components.values()) {
			if (candidate->guid() == staticGeometryComponentGuid) {
				target_._staticGeometryComponent = static_cast<ptr<StaticGeometryComponent>>(candidate);
				break;
			}
		}
	}
}