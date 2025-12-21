#include "LevelAsset.hpp"

#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ::hg::engine::serialization;
using namespace ::hg::engine::assets;
using namespace ::hg;

template <>
void access::Structure<LevelAsset>::serialize(const LevelAsset& self_, mref<StructScopedSlot> record_) {

	record_.insertSlot<String>("levelData") << self_._levelData.encode();

	/**/

	access::Structure<Asset>::serialize(self_, ::hg::move(record_));
}

template <>
void access::Structure<LevelAsset>::hydrate(cref<StructScopedSlot> record_, LevelAsset& target_) {

	access::Structure<Asset>::hydrate(record_, target_);

	/**/

	String levelData {};
	record_.getSlot<String>("levelData") >> levelData;
	if (levelData.empty()) {
		target_._levelData = {};

	} else if (levelData.starts_with(storage::ArchiveScheme)) {

		// Note: "archive://xxxx-xx-xx-xxxxxxxx" -> "xxxx-xx-xx-xxxxxxxx"
		const auto archiveGuid = decodeGuid4228({ levelData.begin() + (storage::ArchiveScheme.size() + 3), levelData.end() });
		target_._levelData = storage::ArchiveUrl { resource::ArchiveGuid::from(archiveGuid) };

	} else {
		::hg::panic();
	}
}
