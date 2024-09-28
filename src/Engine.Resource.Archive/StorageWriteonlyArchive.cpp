#include "StorageWriteonlyArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

StorageWriteonlyArchive::StorageWriteonlyArchive(
	mref<storage::AccessBlobReadWrite> storage_,
	mref<streamoff> baseOffset_,
	mref<streamsize> initialSize_
) :
	MutableStorageArchive(
		std::move(storage_),
		std::move(baseOffset_),
		std::move(initialSize_)
	) {}

StorageWriteonlyArchive::~StorageWriteonlyArchive() = default;

void StorageWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eLoad) [[unlikely]]
	{
		return;
	}

	const auto wrote = _storage->fully().write(_baseOffset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

	if (wrote.empty()) {
		setError();
	} else {
		_cursor += wrote.size();
		_size = std::max(_cursor, _size);
	}
}
