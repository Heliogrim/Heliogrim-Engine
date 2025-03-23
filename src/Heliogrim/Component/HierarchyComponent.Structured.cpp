#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>

#include "HierarchyComponent.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<HierarchyComponent>::serialize(
		const HierarchyComponent& self_,
		mref<StructScopedSlot> slot_
	) {

		slot_.insertSlot<u64>(Symbols::Type) << HierarchyComponent::typeId.data;

		/**/

		Structure<math::Bounding>::serialize(self_._boundaries, slot_.insertStructSlot("boundaries"));
		Structure<math::Transform>::serialize(self_._localTransform, slot_.insertStructSlot("localTransform"));
	}

	template <>
	void access::Structure<HierarchyComponent>::hydrate(
		cref<StructScopedSlot> slot_,
		ref<HierarchyComponent> target_
	) {
		Structure<math::Bounding>::hydrate(slot_.getStructSlot("boundaries"), target_._boundaries);
		Structure<math::Transform>::hydrate(slot_.getStructSlot("localTransform"), target_._localTransform);
	}
}
