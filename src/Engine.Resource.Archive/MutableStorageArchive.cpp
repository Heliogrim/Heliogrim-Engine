#include "MutableStorageArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

s64 MutableStorageArchive::totalSize() const noexcept {
	return _storage->fully().size() - _offset;
}
