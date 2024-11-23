#include "PackageReadWriteArchive.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Package/ReadWritePackage.hpp>

using namespace hg::engine::resource;
using namespace hg;

PackageReadWriteArchive::PackageReadWriteArchive(
	ref<ReadWritePackage> package_,
	ref<package::ReadWriteLinkedArchive> linked_
) :
	InheritMeta(
		package_.getLinker().getStorage(),
		linked_.data.offset,
		streamsize {}/*linked_.data.size*/
	),
	_package(package_),
	_linked(linked_),
	_buffer() {}

cref<ReadWritePackage> PackageReadWriteArchive::getPackage() const noexcept {
	return _package;
}

ref<package::ReadWriteLinkedArchive> PackageReadWriteArchive::getLinked() noexcept {
	return _linked;
}

void PackageReadWriteArchive::seek(s64 pos_) {
	InheritMeta<PackageReadWriteArchive, MutableStorageArchive>::seek(pos_);
}

s64 PackageReadWriteArchive::tell() const noexcept {
	return InheritMeta<PackageReadWriteArchive, MutableStorageArchive>::tell();
}

streamsize PackageReadWriteArchive::totalSize() const noexcept {
	return static_cast<streamsize>(_buffer.size());
}

void PackageReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

	if (size_ <= 0 || hasError()) [[unlikely]] {
		return;
	}

	if (mode_ == ArchiveStreamMode::eStore) {

		const s64 overflow { _cursor + static_cast<s64>(size_) - static_cast<s64>(_buffer.size()) };
		if (overflow > 0) {
			const auto nextSize { _buffer.size() + overflow };
			_buffer.resize(nextSize);
		}

		std::memcpy(&_buffer[_cursor], value_, size_);
		_cursor += static_cast<s64>(size_);

	} else /* if (mode_ == ArchiveStreamMode::eOut) */ {

		if (_cursor + size_ <= _buffer.size()) {
			std::memcpy(value_, &_buffer[_cursor], size_);
			_cursor += static_cast<s64>(size_);

		} else [[unlikely]] {
			setError();
		}

	}
}

void PackageReadWriteArchive::preload(streamsize size_) {

	_buffer.resize(size_, _::byte {});

	/**/

	constexpr streamoff contentOffset = sizeof(PackageHeader);
	auto result = _storage.read(contentOffset + _linked.data.offset, _buffer.getByteSpan());
	::hg::assertrt(result.size() >= size_);

	_size = size_;
	_cursor = streamoff {};
}
