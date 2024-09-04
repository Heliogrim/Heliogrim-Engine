#include "TextureAsset.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::assets;
using namespace hg;

template <>
void access::Structure<TextureAsset>::serialize(const TextureAsset* const self_, mref<RecordScopedSlot> slot_) {

	auto slot = slot_.intoStruct();

	Structure<Guid>::serialize(&self_->_guid, slot.insertSlot<void>("__guid__"));
	slot.insertSlot<u64>("__type__") << self_->_type.data;
	slot.insertSlot<string>("name") << self_->_assetName;

	Structure<Guid>::serialize(&self_->_baseImage, slot.insertSlot<void>("baseImage")); {
		auto images = slot.insertSlot<void>("images").intoSeq();

		for (const auto& entry : self_->_images) {
			Structure<Guid>::serialize(&entry, images.addRecordSlot());
		}
	}

	Structure<math::uivec3>::serialize(&self_->_extent, slot.insertRecordSlot("extent"));
	slot.insertSlot<gfx::TextureFormat>("format") << self_->_format;
	slot.insertSlot<u32>("mipLevel") << self_->_mipLevel;
	slot.insertSlot<gfx::TextureType>("textureType") << self_->_textureType;
}

template <>
void access::Structure<TextureAsset>::deserialize(TextureAsset* const self_, mref<RecordScopedSlot> slot_) {

	const auto slot = slot_.asStruct();

	Structure<Guid>::deserialize(&self_->_guid, slot.getSlot<void>("__guid__"));
	slot.getSlot<u64>("__type__") >> self_->_type.data;
	slot.getSlot<string>("name") >> self_->_assetName;

	/**/

	Structure<Guid>::deserialize(&self_->_baseImage, slot.getSlot<void>("baseImage"));

	/**/

	{
		const auto images = slot.getSlot<void>("images").asSeq();
		const auto count = images.getRecordCount();

		self_->_images.reserve(count);
		for (s64 i = 0; i < count; ++i) {
			Guid tmp {};
			Structure<Guid>::deserialize(&tmp, images.getRecordSlot(i));

			self_->_images.push_back(static_cast<mref<asset_guid>>(tmp));
		}
	}

	Structure<math::uivec3>::deserialize(&self_->_extent, slot.getRecordSlot("extent"));
	slot.getSlot<gfx::TextureFormat>("format") >> self_->_format;
	slot.getSlot<u32>("mipLevel") >> self_->_mipLevel;
	slot.getSlot<gfx::TextureType>("textureType") >> self_->_textureType;
}
