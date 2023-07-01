#include "GfxMaterialPrototype.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<GfxMaterialPrototype>::serialize(
    const GfxMaterialPrototype* const self_,
    mref<RecordScopedSlot> slot_
) {

    auto root = slot_.intoStruct();

    Structure<Guid>::serialize(&self_->_guid, root.insertSlot<void>("__guid__"));
    root.insertSlot<u64>("__type__") << self_->_type.data;
    root.insertSlot<string>("name") << self_->_assetName;

    /**/

    // TODO:
}

template <>
void access::Structure<GfxMaterialPrototype>::deserialize(
    GfxMaterialPrototype* const self_,
    mref<RecordScopedSlot> slot_
) {

    const auto root = slot_.intoStruct();

    Structure<Guid>::deserialize(&self_->_guid, root.getSlot<void>("__guid__"));
    root.getSlot<u64>("__type__") >> self_->_type.data;
    root.getSlot<string>("name") >> self_->_assetName;

    /**/

    // TODO:
}
