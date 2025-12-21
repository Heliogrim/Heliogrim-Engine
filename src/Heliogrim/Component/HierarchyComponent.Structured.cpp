#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>

#include "ComponentGuid.hpp"
#include "HierarchyComponent.hpp"

namespace hg::engine::serialization {
	template <>
	void access::Structure<HierarchyComponent>::serialize(
		const HierarchyComponent& self_,
		mref<StructScopedSlot> record_
	) {
		Structure<math::Bounding>::serialize(self_._boundaries, record_.insertStructSlot("boundaries"));
		Structure<math::Transform>::serialize(self_._localTransform, record_.insertStructSlot("localTransform"));

		/**/

		return Structure<LogicComponent>::serialize(self_, ::hg::move(record_));
	}

	template <>
	void access::Structure<HierarchyComponent>::hydrate(
		cref<StructScopedSlot> record_,
		ref<HierarchyComponent> target_
	) {
		Structure<LogicComponent>::hydrate(record_, target_);

		/**/

		Structure<math::Bounding>::hydrate(record_.getStructSlot("boundaries"), target_._boundaries);
		Structure<math::Transform>::hydrate(record_.getStructSlot("localTransform"), target_._localTransform);
	}
}