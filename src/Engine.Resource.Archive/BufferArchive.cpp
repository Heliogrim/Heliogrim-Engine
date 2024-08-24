#include "BufferArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

BufferArchive::BufferArchive() :
	AutoArray<_::byte>(),
	MemoryReadWriteArchive(*static_cast<ptr<AutoArray<_::byte>>>(this)) {}
