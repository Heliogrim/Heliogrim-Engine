#include "fwd.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::fvec3>::serialize(const math::fvec3* const self_, mref<RecordScopedSlot> slot_) {
	auto slot = slot_.intoStruct();
	slot.insertSlot<f32>("x") << self_->x;
	slot.insertSlot<f32>("y") << self_->y;
	slot.insertSlot<f32>("z") << self_->z;
}

template <>
void access::Structure<math::fvec3>::deserialize(math::fvec3* const self_, mref<RecordScopedSlot> slot_) {
	const auto slot = slot_.intoStruct();
	slot.getSlot<f32>("x") >> self_->x;
	slot.getSlot<f32>("y") >> self_->y;
	slot.getSlot<f32>("z") >> self_->z;
}
