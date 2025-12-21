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
void access::Structure<TextureAsset>::serialize(const TextureAsset& self_, mref<StructScopedSlot> record_) {

	Structure<Guid>::serialize(self_._baseImage, record_.insertStructSlot("baseImage")); {
		auto images = record_.insertSlot<void>("images").intoSeq();

		for (const auto& entry : self_._images) {
			Structure<Guid>::serialize(entry, images.addRecordSlot().intoStruct());
		}
	}

	Structure<math::uivec3>::serialize(self_._extent, record_.insertStructSlot("extent"));
	record_.insertSlot<gfx::TextureFormat>("format") << self_._format;
	record_.insertSlot<u32>("mipLevel") << self_._mipLevel;
	record_.insertSlot<gfx::TextureType>("textureType") << self_._textureType;

	/**/

	return access::Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<TextureAsset>::hydrate(cref<StructScopedSlot> record_, TextureAsset& target_) {

	access::Structure<Asset>::hydrate(record_, target_);

	/**/

	Structure<Guid>::hydrate(record_.getSlot<void>("baseImage").asStruct(), target_._baseImage);

	/**/

	{
		const auto images = record_.getSlot<void>("images").asSeq();
		const auto count = images.getRecordCount();

		target_._images.reserve(count);
		for (s64 i = 0; i < count; ++i) {
			Guid tmp {};
			Structure<Guid>::hydrate(images.getRecordSlot(i).asStruct(), tmp);

			target_._images.push_back(static_cast<mref<AssetGuid>>(tmp));
		}
	}

	Structure<math::uivec3>::hydrate(record_.getRecordSlot("extent").asStruct(), target_._extent);
	record_.getSlot<gfx::TextureFormat>("format") >> target_._format;
	record_.getSlot<u32>("mipLevel") >> target_._mipLevel;
	record_.getSlot<gfx::TextureType>("textureType") >> target_._textureType;
}