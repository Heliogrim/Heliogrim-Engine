#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "PointLightActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<PointLightActor>::serialize(
		const PointLightActor& self_,
		mref<StructScopedSlot> record_
	) {

		insert_named_guid_slot(record_, "point"sv, self_._pointLightComponent->guid());

		/**/

		access::Structure<Actor>::serialize(self_, ::hg::move(record_));
	}

	template <>
	void access::Structure<PointLightActor>::hydrate(
		cref<StructScopedSlot> record_,
		ref<PointLightActor> target_
	) {

		access::Structure<Actor>::hydrate(record_, target_);

		/**/

		const auto pointLightGuid = get_named_guid_slot(record_, "point"sv);
		for (auto* const candidate : target_._components.values()) {
			if (candidate->guid() == pointLightGuid) {
				target_._pointLightComponent = static_cast<ptr<PointLightComponent>>(candidate);
				break;
			}
		}
	}
}
