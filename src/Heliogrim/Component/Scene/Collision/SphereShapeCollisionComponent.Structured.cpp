#include <Engine.Common/Move.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "SphereShapeCollisionComponent.hpp"

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<SphereShapeCollisionComponent>::serialize(
	cref<SphereShapeCollisionComponent> self_,
	mref<StructScopedSlot> record_
) {

	record_.insertSlot<bool>("isStatic") << static_cast<bool>(self_.settings.isStatic);
	record_.insertSlot<bool>("isTrigger") << static_cast<bool>(self_.settings.isTrigger);
	record_.insertSlot<f32>("radius") << self_.radius.at<0>();

	/**/

	return Structure<HierarchyComponent>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<SphereShapeCollisionComponent>::hydrate(cref<StructScopedSlot> record_, SphereShapeCollisionComponent& target_) {

	Structure<HierarchyComponent>::hydrate(record_, target_);

	/**/

	// Bitfields cannot bind to the slot reference.
	auto flag = false;
	record_.getSlot<bool>("isStatic") >> flag;
	target_.settings.isStatic = flag;
	record_.getSlot<bool>("isTrigger") >> flag;
	target_.settings.isTrigger = flag;
	record_.getSlot<f32>("radius") >> target_.radius.at<0>();
}
