#include "StorageReadWriteArchive.hpp"

#include <Engine.Resource.Blob/Blob.hpp>

using namespace hg::engine::resource;
using namespace hg;

StorageReadWriteArchive::StorageReadWriteArchive(
	ref<Blob> storage_,
	const streamoff baseOffset_,
	const streamsize initialSize_
) :
	MutableStorageArchive(
		storage_,
		baseOffset_,
		initialSize_
	) {}

StorageReadWriteArchive::~StorageReadWriteArchive() = default;

void StorageReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError()) [[unlikely]]
	{
		return;
	}

	if (mode_ == ArchiveStreamMode::eStore) {

		const auto wrote = _storage.write(_baseOffset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

		if (wrote.empty()) {
			setError();
		} else {
			_cursor += wrote.size();
			_size = std::max(_cursor, _size);
		}

	} else {

		const auto read = _storage.read(_baseOffset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

		// TODO: Check whether we still need to check for partial read operations
		if (read.empty() || read.size() != size_) {
			setError();
		} else {
			_cursor += read.size();
		}
	}
}
