#include "fwd.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::fvec3>::serialize(const math::vec3_t<float>& self_, mref<StructScopedSlot> record_) {
	record_.insertSlot<f32>("x") << self_.x;
	record_.insertSlot<f32>("y") << self_.y;
	record_.insertSlot<f32>("z") << self_.z;
}

template <>
void access::Structure<math::fvec3>::hydrate(cref<StructScopedSlot> record_, math::vec3_t<float>& target_) {
	record_.getSlot<f32>("x") >> target_.x;
	record_.getSlot<f32>("y") >> target_.y;
	record_.getSlot<f32>("z") >> target_.z;
}