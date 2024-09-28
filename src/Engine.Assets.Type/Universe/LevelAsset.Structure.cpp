#include "LevelAsset.hpp"

#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Engine.Serialization/Structure/SliceScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ::hg::engine::serialization;
using namespace ::hg::engine::assets;
using namespace ::hg;

template <>
void access::Structure<LevelAsset>::serialize(const LevelAsset& self_, mref<StructScopedSlot> slot_) {

	Structure<Guid>::serialize(self_._guid, slot_.insertStructSlot("__guid__"));
	slot_.insertSlot<u64>("__type__") << self_._type.data;
	slot_.insertSlot<string>("name") << self_._assetName;

	/**/

	slot_.insertSlot<String>("levelData") << self_._levelData.encode();
}

template <>
void access::Structure<LevelAsset>::hydrate(cref<StructScopedSlot> slot_, LevelAsset& target_) {

	Structure<Guid>::hydrate(slot_.getStructSlot("__guid__"), target_._guid);
	slot_.getSlot<u64>("__type__") >> target_._type.data;
	slot_.getSlot<string>("name") >> target_._assetName;

	::hg::assertd(target_._type.data == LevelAsset::typeId.data);

	/**/

	String levelData {};
	slot_.getSlot<String>("levelData") >> levelData;
	if (levelData.empty()) {
		target_._levelData = {};

	} else if (levelData.starts_with(storage::ArchiveScheme)) {

		// Note: "archive://xxxx-xx-xx-xxxxxxxx" -> "xxxx-xx-xx-xxxxxxxx"
		auto archiveGuid = decodeGuid4228({ levelData.begin() + (storage::ArchiveScheme.size() + 3), levelData.end() });
		target_._levelData = storage::ArchiveUrl { std::move(archiveGuid) };

	} else {
		::hg::panic();
	}
}
