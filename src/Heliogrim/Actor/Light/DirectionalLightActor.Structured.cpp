#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "DirectionalLightActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<DirectionalLightActor>::serialize(
		const DirectionalLightActor& self_,
		mref<StructScopedSlot> record_
	) {

		insert_named_guid_slot(record_, "directional"sv, self_._directionalLightComponent->getComponentGuid());

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

		const auto directionalLightGuid = get_named_guid_slot(record_, "directional"sv);
		for (auto* const candidate : target_._components.values()) {
			if (candidate->getComponentGuid() == directionalLightGuid) {
				target_._directionalLightComponent = static_cast<ptr<DirectionalLightComponent>>(candidate);
				break;
			}
		}
	}
}