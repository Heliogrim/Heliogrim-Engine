#include <Engine.Common/Guid.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/__fwd.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Guid>::serialize(const Guid* const self_, mref<RecordScopedSlot> slot_) {

    auto slot = slot_.intoStruct();

    slot.insertSlot<decltype(Guid::pre)>("pre") << self_->pre;
    slot.insertSlot<decltype(Guid::c0)>("c0") << self_->c0;
    slot.insertSlot<decltype(Guid::c1)>("c1") << self_->c1;
    slot.insertSlot<decltype(Guid::post)>("post") << self_->post;
}

template <>
void access::Structure<Guid>::deserialize(Guid* const self_, cref<RecordScopedSlot> slot_) {

    const auto slot = slot_.intoStruct();

    slot.getSlot<decltype(Guid::pre)>("pre") >> self_->pre;
    slot.getSlot<decltype(Guid::c0)>("c0") >> self_->c0;
    slot.getSlot<decltype(Guid::c1)>("c1") >> self_->c1;
    slot.getSlot<decltype(Guid::post)>("post") >> self_->post;
}
