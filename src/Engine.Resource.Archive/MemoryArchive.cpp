#include "MemoryArchive.hpp"

using namespace hg::engine::resource;
using namespace hg;

MemoryBaseArchive::MemoryBaseArchive() :
	Archive(),
	_pos(0LL) {}

void MemoryBaseArchive::seek(const s64 pos_) {
	_pos = pos_;
}

s64 MemoryBaseArchive::tell() const noexcept {
	return _pos;
}
