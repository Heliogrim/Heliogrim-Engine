#include "fwd.hpp"

#include <bit>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Math/Transform.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ::hg::engine::serialization;
using namespace ::hg;

template <>
void access::Structure<math::Transform>::serialize(const math::Transform& self_, mref<StructScopedSlot> slot_) {
	Structure<math::fvec3>::serialize(
		*std::bit_cast<const math::fvec3*>(&self_._location),
		slot_.insertRecordSlot("location").intoStruct()
	);
	Structure<math::fvec3>::serialize(
		*std::bit_cast<const math::fvec3*>(&self_._rotator),
		slot_.insertRecordSlot("rotator").intoStruct()
	);
	Structure<math::fvec3>::serialize(
		self_._scale,
		slot_.insertRecordSlot("scale").intoStruct()
	);
}

template <>
void access::Structure<math::Transform>::hydrate(cref<StructScopedSlot> slot_, math::Transform& target_) {
	Structure<math::fvec3>::hydrate(
		slot_.getRecordSlot("location").asStruct(),
		*std::bit_cast<math::fvec3*>(&target_._location)
	);
	Structure<math::fvec3>::hydrate(
		slot_.getRecordSlot("rotator").asStruct(),
		*std::bit_cast<math::fvec3*>(&target_._rotator)
	);
	Structure<math::fvec3>::hydrate(
		slot_.getRecordSlot("scale").asStruct(),
		target_._scale
	);
}
