#include "StorageReadWriteArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

StorageReadWriteArchive::StorageReadWriteArchive(
	mref<storage::AccessBlobReadWrite> storage_,
	mref<streamoff> offset_,
	mref<streamsize> capacity_
) :
	MutableStorageArchive(
		std::move(storage_),
		std::move(offset_),
		std::move(capacity_)
	) {}

StorageReadWriteArchive::~StorageReadWriteArchive() = default;

void StorageReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError()) [[unlikely]]
	{
		return;
	}

	if (mode_ == ArchiveStreamMode::eStore) {

		const auto wrote = _storage->fully().write(_offset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

		if (wrote.empty()) {
			setError();
		} else {
			_cursor += wrote.size();
		}

	} else {

		const auto read = _storage->fully().read(_offset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

		// TODO: Check whether we still need to check for partial read operations
		if (read.empty() || read.size() != size_) {
			setError();
		} else {
			_cursor += read.size();
		}
	}
}
