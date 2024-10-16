#include "PackageStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

PackageStorage::PackageStorage(
	mref<Arci<IStorage>> backing_,
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
	_backing(std::move(backing_)) {
	// Warning: Just a temporary explicit error
	::hg::assertrt(_backing == nullptr || _backing->isPrimaryStorage());
}

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

Arci<engine::storage::IStorage> PackageStorage::getBacking() const noexcept {
	return _backing;
}
