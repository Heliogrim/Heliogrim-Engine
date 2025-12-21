#include "fwd.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::ivec3>::serialize(const math::vec3_t<int>& self_, mref<StructScopedSlot> record_) {
	record_.insertSlot<s32>("x") << self_.x;
	record_.insertSlot<s32>("y") << self_.y;
	record_.insertSlot<s32>("z") << self_.z;
}

template <>
void access::Structure<math::ivec3>::hydrate(cref<StructScopedSlot> record_, math::vec3_t<int>& target_) {
	record_.getSlot<s32>("x") >> target_.x;
	record_.getSlot<s32>("y") >> target_.y;
	record_.getSlot<s32>("z") >> target_.z;
}