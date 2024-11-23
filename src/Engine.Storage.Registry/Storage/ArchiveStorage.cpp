#include "ArchiveStorage.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg::engine::storage::system;
using namespace hg;

ArchiveStorage::ArchiveStorage(
	mref<Arci<IStorage>> backing_,
	mref<resource::ArchiveGuid> archiveGuid_,
	bool readable_,
	bool writeable_,
	bool randomReadable_,
	bool randomWritable_
) noexcept :
	InheritMeta(),
	_readable(readable_),
	_writeable(writeable_),
	_randomReadable(randomReadable_),
	_randomWritable(randomWritable_),
	_backing(::hg::move(backing_)),
	_archiveGuid(::hg::move(archiveGuid_)) {}

bool ArchiveStorage::isReadable() const noexcept {
	return _readable;
}

bool ArchiveStorage::isWriteable() const noexcept {
	return _writeable;
}

bool ArchiveStorage::isRandomAccess() const noexcept {
	return _randomReadable || _randomWritable;
}

bool ArchiveStorage::isRandomReadable() const noexcept {
	return _randomReadable;
}

bool ArchiveStorage::isRandomWritable() const noexcept {
	return _randomWritable;
}

Arci<engine::storage::IStorage> ArchiveStorage::getBacking() const noexcept {
	return _backing;
}

engine::resource::ArchiveGuid ArchiveStorage::getArchiveGuid() const noexcept {
	return _archiveGuid;
}
