#include "PackageStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

PackageStorage::PackageStorage() noexcept :
	IStorage(),
	_readable(),
	_writeable(),
	_randomReadable(),
	_randomWritable() {}

PackageStorage::PackageStorage(
	bool readable_,
	bool writeable_,
	bool randomReadable_,
	bool randomWritable_
) noexcept :
	IStorage(),
	_readable(readable_),
	_writeable(writeable_),
	_randomReadable(randomReadable_),
	_randomWritable(randomWritable_) {}

bool PackageStorage::isReadable() const noexcept {
	return _readable;
}

bool PackageStorage::isWriteable() const noexcept {
	return _writeable;
}

bool PackageStorage::isRandomAccess() const noexcept {
	return _randomReadable || _randomWritable;
}

bool PackageStorage::isRandomReadable() const noexcept {
	return _randomReadable;
}

bool PackageStorage::isRandomWritable() const noexcept {
	return _randomWritable;
}
