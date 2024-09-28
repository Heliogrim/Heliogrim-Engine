#include "fwd.hpp"

#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<math::uivec3>::serialize(const math::vec3_t<u32>& self_, mref<StructScopedSlot> slot_) {
	slot_.insertSlot<u32>("x") << self_.x;
	slot_.insertSlot<u32>("y") << self_.y;
	slot_.insertSlot<u32>("z") << self_.z;
}

template <>
void access::Structure<math::uivec3>::hydrate(cref<StructScopedSlot> slot_, math::vec3_t<u32>& target_) {
	slot_.getSlot<u32>("x") >> target_.x;
	slot_.getSlot<u32>("y") >> target_.y;
	slot_.getSlot<u32>("z") >> target_.z;
}
