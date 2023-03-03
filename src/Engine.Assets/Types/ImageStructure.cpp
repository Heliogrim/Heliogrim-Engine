#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

#include "Image.hpp"

using namespace hg::engine::assets;
using namespace hg::engine::serialization;
using namespace hg;

template <>
void access::Structure<Image>::serialize(const Image* const self_, mref<RecordScopedSlot> slot_) {

    auto slot = slot_.intoStruct();

    Structure<Guid>::serialize(&self_->_guid, slot.insertSlot<void>("__guid__"));
    slot.insertSlot<u64>("__type__") << self_->_type.data;
    slot.insertSlot<string>("name") << self_->_assetName;

    Vector<string> sources {};
    sources.reserve(self_->_sources.size());

    for (const auto& entry : self_->_sources) {
        sources.push_back(string { entry.encode() });
    }

    slot.insertSlot<string, Vector>("sources") << sources;
}

template <>
void access::Structure<Image>::deserialize(Image* const self_, mref<RecordScopedSlot> slot_) {

    const auto slot = slot_.intoStruct();

    Structure<Guid>::deserialize(&self_->_guid, slot.getSlot<void>("__guid__"));
    slot.getSlot<u64>("__type__") >> self_->_type.data;
    slot.getSlot<string>("name") >> self_->_assetName;

    Vector<string> sources {};
    slot.getSlot<string, Vector>("sources") >> sources;

    self_->_sources.reserve(sources.size());
    for (const auto& entry : sources) {

        if (entry.size() < 8ui64) {
            continue;
        }

        self_->_sources.push_back(Url { "file"sv, entry.substr(7ui64) });
    }

}
