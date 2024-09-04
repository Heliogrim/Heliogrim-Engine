#include "fwd.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::ivec3>::serialize(const math::ivec3* const self_, mref<RecordScopedSlot> slot_) {
	auto slot = slot_.intoStruct();
	slot.insertSlot<s32>("x") << self_->x;
	slot.insertSlot<s32>("y") << self_->y;
	slot.insertSlot<s32>("z") << self_->z;
}

template <>
void access::Structure<math::ivec3>::deserialize(math::ivec3* const self_, mref<RecordScopedSlot> slot_) {
	const auto slot = slot_.intoStruct();
	slot.getSlot<s32>("x") >> self_->x;
	slot.getSlot<s32>("y") >> self_->y;
	slot.getSlot<s32>("z") >> self_->z;
}
