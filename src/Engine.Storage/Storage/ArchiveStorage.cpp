#include "ArchiveStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

ArchiveStorage::ArchiveStorage(
	mref<Arci<IStorage>> backing_,
	bool readable_,
	bool writeable_,
	bool randomReadable_,
	bool randomWritable_
) noexcept :
	IStorage(),
	_readable(readable_),
	_writeable(writeable_),
	_randomReadable(randomReadable_),
	_randomWritable(randomWritable_),
	_backing(std::move(backing_)) {
	// Warning: Just a temporary explicit error
	::hg::assertrt(_backing == nullptr || _backing->isPrimaryStorage());
}

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
