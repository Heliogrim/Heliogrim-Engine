#include "DirectionalLightActor.hpp"

#include <Engine.Serialization.Structures/Common/Guid.hpp>

import Engine.Serialization;

namespace hg::engine::serialization {
	template <>
	void access::Structure<DirectionalLightActor>::serialize(
		const DirectionalLightActor& self_,
		mref<StructScopedSlot> record_
	) {

		insert_named_guid_slot(record_, "directional"sv, self_._directionalLightComponent->guid());

		/**/

		access::Structure<Actor>::serialize(self_, ::hg::move(record_));
	}

	template <>
	void access::Structure<DirectionalLightActor>::hydrate(
		cref<StructScopedSlot> record_,
		ref<DirectionalLightActor> target_
	) {

		access::Structure<Actor>::hydrate(record_, target_);

		/**/

		const auto directionalLightGuid = ComponentGuid { get_named_guid_slot(record_, "directional"sv) };
		for (auto* const candidate : target_._components.values()) {
			if (candidate->guid() == directionalLightGuid) {
				target_._directionalLightComponent = static_cast<ptr<DirectionalLightComponent>>(candidate);
				break;
			}
		}
	}
}
