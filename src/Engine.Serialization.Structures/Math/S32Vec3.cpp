#include "fwd.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::ivec3>::serialize(const math::vec3_t<int>& self_, mref<StructScopedSlot> slot_) {
	slot_.insertSlot<s32>("x") << self_.x;
	slot_.insertSlot<s32>("y") << self_.y;
	slot_.insertSlot<s32>("z") << self_.z;
}

template <>
void access::Structure<math::ivec3>::hydrate(cref<StructScopedSlot> slot_, math::vec3_t<int>& target_) {
	slot_.getSlot<s32>("x") >> target_.x;
	slot_.getSlot<s32>("y") >> target_.y;
	slot_.getSlot<s32>("z") >> target_.z;
}
