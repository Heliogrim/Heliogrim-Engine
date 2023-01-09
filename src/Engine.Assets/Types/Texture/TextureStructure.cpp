#include "Texture.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ember::engine::serialization;
using namespace ember::engine::assets;
using namespace ember;

template <>
void access::Structure<math::uivec3>::serialize(const math::uivec3* const self_, mref<RecordScopedSlot> slot_) {
    auto slot = slot_.intoStruct();
    slot.insertSlot<u32>("x") << self_->x;
    slot.insertSlot<u32>("y") << self_->y;
    slot.insertSlot<u32>("z") << self_->z;
}

template <>
void access::Structure<math::uivec3>::deserialize(math::uivec3* const self_, cref<RecordScopedSlot> slot_) {
    const auto slot = slot_.intoStruct();
    slot.getSlot<u32>("x") >> self_->x;
    slot.getSlot<u32>("y") >> self_->y;
    slot.getSlot<u32>("z") >> self_->z;
}

template <>
void access::Structure<Texture>::serialize(const Texture* const self_, mref<RecordScopedSlot> slot_) {

    auto slot = slot_.intoStruct();

    slot.insertSlot<u64>("__guid__") << self_->_guid.as_uint64();
    slot.insertSlot<u64>("__type__") << self_->_type.data;
    slot.insertSlot<string>("name") << self_->_assetName;

    slot.insertSlot<u64>("baseImage") << self_->_baseImage.as_uint64();

    Vector<u64> images {};
    images.reserve(self_->_images.size());

    for (const auto& entry : self_->_images) {
        images.push_back(entry.as_uint64());
    }

    slot.insertSlot<u64, Vector>("images") << images;

    Structure<math::uivec3>::serialize(&self_->_extent, slot.insertRecordSlot("extent"));
    slot.insertSlot<gfx::TextureFormat>("format") << self_->_format;
    slot.insertSlot<u32>("mipLevel") << self_->_mipLevel;
    slot.insertSlot<gfx::TextureType>("textureType") << self_->_textureType;
}

template <>
void access::Structure<Texture>::deserialize(Texture* const self_, cref<RecordScopedSlot> slot_) {

    const auto slot = slot_.intoStruct();

    slot.getSlot<u64>("__guid__") >> self_->_guid.as_uint64();
    slot.getSlot<u64>("__type__") >> self_->_type.data;
    slot.getSlot<string>("name") >> self_->_assetName;

    slot.getSlot<u64>("baseImage") >> self_->_baseImage.as_uint64();

    Vector<u64> images {};
    slot.getSlot<u64, Vector>("images") >> images;

    self_->_images.reserve(images.size());
    for (const auto& entry : images) {
        self_->_images.push_back(asset_guid::from(entry));
    }

    Structure<math::uivec3>::deserialize(&self_->_extent, slot.getRecordSlot("extent"));
    slot.getSlot<gfx::TextureFormat>("format") >> self_->_format;
    slot.getSlot<u32>("mipLevel") >> self_->_mipLevel;
    slot.getSlot<gfx::TextureType>("textureType") >> self_->_textureType;
}
