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
void access::Structure<TextureAsset>::serialize(const TextureAsset& self_, mref<StructScopedSlot> slot_) {

	Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot("__guid__"));
	slot_.insertSlot<u64>("__type__") << self_._type.data;
	slot_.insertSlot<string>("name") << self_._assetName;

	Structure<Guid>::serialize(self_._baseImage, slot_.insertStructSlot("baseImage")); {
		auto images = slot_.insertSlot<void>("images").intoSeq();

		for (const auto& entry : self_._images) {
			Structure<Guid>::serialize(entry, images.addRecordSlot().intoStruct());
		}
	}

	Structure<math::uivec3>::serialize(self_._extent, slot_.insertStructSlot("extent"));
	slot_.insertSlot<gfx::TextureFormat>("format") << self_._format;
	slot_.insertSlot<u32>("mipLevel") << self_._mipLevel;
	slot_.insertSlot<gfx::TextureType>("textureType") << self_._textureType;
}

template <>
void access::Structure<TextureAsset>::hydrate(cref<StructScopedSlot> slot_, TextureAsset& target_) {

	Structure<Guid>::hydrate(slot_.getStructSlot("__guid__"), target_._guid);
	slot_.getSlot<u64>("__type__") >> target_._type.data;
	slot_.getSlot<string>("name") >> target_._assetName;

	/**/

	Structure<Guid>::hydrate(slot_.getSlot<void>("baseImage").asStruct(), target_._baseImage);

	/**/

	{
		const auto images = slot_.getSlot<void>("images").asSeq();
		const auto count = images.getRecordCount();

		target_._images.reserve(count);
		for (s64 i = 0; i < count; ++i) {
			Guid tmp {};
			Structure<Guid>::hydrate(images.getRecordSlot(i).asStruct(), tmp);

			target_._images.push_back(static_cast<mref<AssetGuid>>(tmp));
		}
	}

	Structure<math::uivec3>::hydrate(slot_.getRecordSlot("extent").asStruct(), target_._extent);
	slot_.getSlot<gfx::TextureFormat>("format") >> target_._format;
	slot_.getSlot<u32>("mipLevel") >> target_._mipLevel;
	slot_.getSlot<gfx::TextureType>("textureType") >> target_._textureType;
}
