#include "StorageReadonlyArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

StorageReadonlyArchive::StorageReadonlyArchive(
	mref<storage::AccessBlobReadonly> storage_,
	mref<streamoff> offset_,
	mref<streamsize> capacity_
) :
	ImmutableStorageArchive(
		std::move(storage_),
		std::move(offset_),
		std::move(capacity_)
	) {}

StorageReadonlyArchive::~StorageReadonlyArchive() = default;

void StorageReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eStore) [[unlikely]]
	{
		return;
	}

	const auto read = _storage->fully().read(_offset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

	// TODO: Check whether we still need to check for partial read operations
	if (read.empty() || read.size() != size_) {
		setError();
	} else {
		_cursor += static_cast<streamoff>(read.size());
	}
}
