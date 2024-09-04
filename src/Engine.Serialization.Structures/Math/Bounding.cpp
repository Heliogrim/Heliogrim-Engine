#include "fwd.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ::hg::engine::serialization;
using namespace ::hg;

template <>
void access::Structure<math::Bounding>::serialize(const math::Bounding* const self_, mref<RecordScopedSlot> slot_) {
	auto slot = slot_.intoStruct();
	Structure<math::fvec3>::serialize(&self_->_center, slot.insertRecordSlot("center"));
	Structure<math::fvec3>::serialize(&self_->_extent, slot.insertRecordSlot("extent"));
	slot.insertSlot<f32>("length") << self_->_extent_length;
}

template <>
void access::Structure<math::Bounding>::deserialize(math::Bounding* const self_, mref<RecordScopedSlot> slot_) {
	const auto slot = slot_.asStruct();
	Structure<math::fvec3>::serialize(&self_->_center, slot.getRecordSlot("center"));
	Structure<math::fvec3>::serialize(&self_->_extent, slot.getRecordSlot("extent"));
	slot.getSlot<f32>("length") >> self_->_extent_length;
}
