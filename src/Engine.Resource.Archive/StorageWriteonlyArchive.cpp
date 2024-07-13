#include "StorageWriteonlyArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

StorageWriteonlyArchive::StorageWriteonlyArchive(
	mref<storage::AccessBlobReadWrite> storage_,
	mref<streamoff> offset_,
	mref<streamsize> capacity_
) :
	MutableStorageArchive(
		std::move(storage_),
		std::move(offset_),
		std::move(capacity_)
	) {}

StorageWriteonlyArchive::~StorageWriteonlyArchive() = default;

void StorageWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eLoad) [[unlikely]]
	{
		return;
	}

	const auto wrote = _storage->fully().write(_offset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

	if (wrote.empty()) {
		setError();
	} else {
		_cursor += wrote.size();
	}
}
