#include "StorageReadonlyArchive.hpp"

#include <Engine.Resource.Blob/Blob.hpp>

using namespace hg::engine::resource;
using namespace hg;

StorageReadonlyArchive::StorageReadonlyArchive(
	cref<Blob> storage_,
	const streamoff baseOffset_,
	const streamsize initialSize_
) :
	InheritMeta(
		storage_,
		baseOffset_,
		initialSize_
	) {}

StorageReadonlyArchive::~StorageReadonlyArchive() = default;

void StorageReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eStore) [[unlikely]]
	{
		return;
	}

	const auto read = _storage.read(_baseOffset + _cursor, { static_cast<ptr<_::byte>>(value_), size_ });

	// TODO: Check whether we still need to check for partial read operations
	if (read.empty() || read.size() != size_) {
		setError();
	} else {
		_cursor += static_cast<streamoff>(read.size());
	}
}
