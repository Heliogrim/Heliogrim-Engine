#include "LocalFileStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

LocalFileStorage::LocalFileStorage(
	mref<fs::Path> lfsPath_,
	const bool readable_,
	const bool writeable_,
	const bool randomReadable_,
	const bool randomWritable_
) noexcept :
	InheritMeta(),
	_readable(readable_),
	_writeable(writeable_),
	_randomReadable(randomReadable_),
	_randomWritable(randomWritable_),
	_lfsPath(std::move(lfsPath_)) {}

bool LocalFileStorage::isReadable() const noexcept {
	return _readable;
}

bool LocalFileStorage::isWriteable() const noexcept {
	return _writeable;
}

bool LocalFileStorage::isRandomAccess() const noexcept {
	return _randomReadable || _randomWritable;
}

bool LocalFileStorage::isRandomReadable() const noexcept {
	return _randomReadable;
}

bool LocalFileStorage::isRandomWritable() const noexcept {
	return _randomWritable;
}
