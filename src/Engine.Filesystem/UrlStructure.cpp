#include "Url.hpp"

#include <Engine.Serialization/Access/Structure.hpp>

#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::fs;
using namespace hg;

template <>
void access::Structure<Url>::serialize(const ptr<const Url> self_, mref<RecordScopedSlot> slot_) {
    auto slot = slot_.intoStruct();
    slot.insertSlot<string>("scheme") << self_->_scheme;
    slot.insertSlot<string>("path") << self_->_path.string();
}

template <>
void access::Structure<Url>::deserialize(const ptr<Url> self_, mref<RecordScopedSlot> slot_) {
    auto slot = slot_.intoStruct();
    slot.getSlot<string>("scheme") >> self_->_scheme;

    string tmp {};
    slot.getSlot<string>("path") >> tmp;
    self_->setPath(_STD move(tmp));
}
