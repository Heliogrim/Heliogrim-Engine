#include "BufferArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

BufferArchive::BufferArchive() :
	MemoryReadWriteArchive(*static_cast<ptr<AutoArray<_::byte>>>(this)),
	AutoArray<_::byte>() {}
