#include <Engine.Common/Move.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "BoxShapeCollisionComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<BoxShapeCollisionComponent>::serialize(cref<BoxShapeCollisionComponent> self_, mref<StructScopedSlot> record_) {

	record_.insertSlot<bool>("isStatic") << static_cast<bool>(self_.settings.isStatic);
	record_.insertSlot<bool>("isTrigger") << static_cast<bool>(self_.settings.isTrigger);
	Structure<math::fvec3>::serialize(self_.extent, record_.insertStructSlot("extent"));

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<BoxShapeCollisionComponent>::hydrate(cref<StructScopedSlot> record_, BoxShapeCollisionComponent& target_) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	// Bitfields cannot bind to the slot reference.
	auto flag = false;
	record_.getSlot<bool>("isStatic") >> flag;
	target_.settings.isStatic = flag;
	record_.getSlot<bool>("isTrigger") >> flag;
	target_.settings.isTrigger = flag;
	Structure<math::fvec3>::hydrate(record_.getStructSlot("extent"), target_.extent);
}
