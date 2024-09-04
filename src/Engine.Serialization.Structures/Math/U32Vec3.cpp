#include "fwd.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::uivec3>::serialize(const math::uivec3* const self_, mref<RecordScopedSlot> slot_) {
	auto slot = slot_.intoStruct();
	slot.insertSlot<u32>("x") << self_->x;
	slot.insertSlot<u32>("y") << self_->y;
	slot.insertSlot<u32>("z") << self_->z;
}

template <>
void access::Structure<math::uivec3>::deserialize(math::uivec3* const self_, mref<RecordScopedSlot> slot_) {
	const auto slot = slot_.intoStruct();
	slot.getSlot<u32>("x") >> self_->x;
	slot.getSlot<u32>("y") >> self_->y;
	slot.getSlot<u32>("z") >> self_->z;
}
