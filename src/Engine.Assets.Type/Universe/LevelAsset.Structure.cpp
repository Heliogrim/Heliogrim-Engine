#include "LevelAsset.hpp"

#include <charconv>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace ::hg::engine::serialization;
using namespace ::hg::engine::assets;
using namespace ::hg;

/* Non-throwing counterpart of decodeGuid4228 ("pre-c0-c1-post", decimal); file data reaches this. */
static Opt<Guid> try_decode_guid4228(StringView str_) {
	Guid guid {};
	const auto part = [&str_](auto& out_, const bool last_) {
		const auto end = last_ ? str_.size() : str_.find('-');
		if (end == StringView::npos || end == 0uLL) {
			return false;
		}
		const auto [ptr, ec] = std::from_chars(str_.data(), str_.data() + end, out_);
		if (ec != std::errc {} || ptr != str_.data() + end) {
			return false;
		}
		str_.remove_prefix(last_ ? end : end + 1uLL);
		return true;
	};
	if (part(guid.pre, false) && part(guid.c0, false) && part(guid.c1, false) && part(guid.post, true)) {
		return Some(guid);
	}
	return None;
}

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
	target_._levelData = {};
	if (levelData.empty()) {
		return;
	}

	// Note: "archive://xxxx-xx-xx-xxxxxxxx" -> "xxxx-xx-xx-xxxxxxxx"
	const auto prefix = storage::ArchiveScheme.size() + 3uLL;
	const auto archiveGuid = levelData.starts_with(storage::ArchiveScheme) && levelData.size() > prefix ?
		try_decode_guid4228(StringView { levelData }.substr(prefix)) :
		None;
	if (not archiveGuid.has_value()) {
		// The value is file data; reject it like any other unreadable record.
		record_.slot()->getState().root->archive->setError();
		return;
	}

	target_._levelData = storage::ArchiveUrl { resource::ArchiveGuid::from(archiveGuid.value()) };
}
