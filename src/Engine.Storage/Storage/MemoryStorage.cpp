#include "MemoryStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

MemoryStorage::MemoryStorage() noexcept :
	IStorage(),
	_readable(),
	_writeable(),
	_randomReadable(),
	_randomWritable() {}

MemoryStorage::MemoryStorage(
	mref<MemoryObject> memoryObject_,
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
	_memory(std::move(memoryObject_)) {}

bool MemoryStorage::isReadable() const noexcept {
	return _readable;
}

bool MemoryStorage::isWriteable() const noexcept {
	return _writeable;
}

bool MemoryStorage::isRandomAccess() const noexcept {
	return _randomReadable || _randomWritable;
}

bool MemoryStorage::isRandomReadable() const noexcept {
	return _randomReadable;
}

bool MemoryStorage::isRandomWritable() const noexcept {
	return _randomWritable;
}
