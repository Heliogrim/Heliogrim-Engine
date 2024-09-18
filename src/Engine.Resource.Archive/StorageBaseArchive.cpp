#include "StorageBaseArchive.hpp"

#include <format>
#include <Engine.Asserts/Panic.hpp>

using namespace hg::engine::resource;
using namespace hg;

StorageBaseArchive::StorageBaseArchive(
	mref<streamoff> baseOffset_,
	mref<streamsize> initialSize_
) :
	_baseOffset(baseOffset_),
	_size(initialSize_),
	_cursor(0) {}

StorageBaseArchive::~StorageBaseArchive() noexcept = default;

void StorageBaseArchive::seek(const s64 pos_) {
	_cursor = pos_;
}

s64 StorageBaseArchive::tell() const noexcept {
	// TODO: Check why tell is not supported?!? return -1;
	return _cursor;
}
