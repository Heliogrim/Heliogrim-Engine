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
void access::Structure<math::Bounding>::serialize(const math::Bounding& self_, mref<StructScopedSlot> record_) {
	Structure<math::fvec3>::serialize(self_._center, record_.insertStructSlot("center"));
	Structure<math::fvec3>::serialize(self_._extent, record_.insertStructSlot("extent"));
	record_.insertSlot<f32>("length") << self_._extent_length;
}

template <>
void access::Structure<math::Bounding>::hydrate(cref<StructScopedSlot> record_, math::Bounding& target_) {
	Structure<math::fvec3>::hydrate(record_.getStructSlot("center"), target_._center);
	Structure<math::fvec3>::hydrate(record_.getStructSlot("extent"), target_._extent);
	record_.getSlot<f32>("length") >> target_._extent_length;
}
