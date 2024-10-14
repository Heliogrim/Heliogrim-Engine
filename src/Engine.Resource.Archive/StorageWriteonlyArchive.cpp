#include "StorageWriteonlyArchive.hpp"

#include <Engine.Resource.Blob/Blob.hpp>

using namespace hg::engine::resource;
using namespace hg;

StorageWriteonlyArchive::StorageWriteonlyArchive(
	ref<Blob> storage_,
	const streamoff baseOffset_,
	const streamsize initialSize_
) :
	MutableStorageArchive(
		storage_,
		baseOffset_,
		initialSize_
	) {}

StorageWriteonlyArchive::~StorageWriteonlyArchive() = default;

void StorageWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eLoad) [[unlikely]]
	{
		return;
	}

	const auto wrote = _storage.write(_baseOffset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

	if (wrote.empty()) {
		setError();
	} else {
		_cursor += wrote.size();
		_size = std::max(_cursor, _size);
	}
}
