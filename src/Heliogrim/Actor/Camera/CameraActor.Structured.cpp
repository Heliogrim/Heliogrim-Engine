#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "CameraActor.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<CameraActor>::serialize(
		const CameraActor& self_,
		mref<StructScopedSlot> record_
	) {

		insert_named_guid_slot(record_, "camera"sv, self_._cameraComponent->getComponentGuid());
		insert_named_guid_slot(record_, "mount"sv, self_._mountComponent->getComponentGuid());

		/**/

		access::Structure<Actor>::serialize(self_, ::hg::move(record_));
	}

	template <>
	void access::Structure<CameraActor>::hydrate(cref<StructScopedSlot> record_, ref<CameraActor> target_) {

		access::Structure<Actor>::hydrate(record_, target_);

		/**/

		const auto cameraComponentGuid = get_named_guid_slot(record_, "camera"sv);
		const auto mountComponentGuid = get_named_guid_slot(record_, "mount"sv);

		for (auto* const candidate : target_._components.values()) {
			if (candidate->getComponentGuid() == cameraComponentGuid) {
				target_._cameraComponent = static_cast<ptr<CameraComponent>>(candidate);
			}
			if (candidate->getComponentGuid() == mountComponentGuid) {
				target_._mountComponent = static_cast<ptr<HierarchyComponent>>(candidate);
			}
		}
	}
}