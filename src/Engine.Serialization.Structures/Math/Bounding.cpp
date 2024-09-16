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
void access::Structure<math::Bounding>::serialize(const math::Bounding& self_, mref<StructScopedSlot> slot_) {
	Structure<math::fvec3>::serialize(self_._center, slot_.insertRecordSlot("center").intoStruct());
	Structure<math::fvec3>::serialize(self_._extent, slot_.insertRecordSlot("extent").intoStruct());
	slot_.insertSlot<f32>("length") << self_._extent_length;
}

template <>
void access::Structure<math::Bounding>::hydrate(cref<StructScopedSlot> slot_, math::Bounding& target_) {
	Structure<math::fvec3>::serialize(target_._center, slot_.getRecordSlot("center").asStruct());
	Structure<math::fvec3>::serialize(target_._extent, slot_.getRecordSlot("extent").asStruct());
	slot_.getSlot<f32>("length") >> target_._extent_length;
}
