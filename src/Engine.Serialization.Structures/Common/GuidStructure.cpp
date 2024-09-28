#include <Engine.Common/Guid.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/__fwd.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Guid>::serialize(const Guid& self_, mref<StructScopedSlot> slot_) {
	slot_.insertSlot<decltype(Guid::pre)>("pre") << self_.pre;
	slot_.insertSlot<decltype(Guid::c0)>("c0") << self_.c0;
	slot_.insertSlot<decltype(Guid::c1)>("c1") << self_.c1;
	slot_.insertSlot<decltype(Guid::post)>("post") << self_.post;
}

template <>
void access::Structure<Guid>::hydrate(cref<StructScopedSlot> slot_, Guid& target_) {
	slot_.getSlot<decltype(Guid::pre)>("pre") >> target_.pre;
	slot_.getSlot<decltype(Guid::c0)>("c0") >> target_.c0;
	slot_.getSlot<decltype(Guid::c1)>("c1") >> target_.c1;
	slot_.getSlot<decltype(Guid::post)>("post") >> target_.post;
}
