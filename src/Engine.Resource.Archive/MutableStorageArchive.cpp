import Engine.Resource.Archive;

using namespace hg::engine::resource;
using namespace hg;

s64 MutableStorageArchive::totalSize() const noexcept {
	return _size;
}
