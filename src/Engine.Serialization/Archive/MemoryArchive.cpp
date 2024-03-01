#include "MemoryArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

MemoryBaseArchive::MemoryBaseArchive() :
    Archive(),
    _pos(0LL) {}

string_view MemoryBaseArchive::getArchiveName() const noexcept {
    return "MemoryArchive"sv;
}

void MemoryBaseArchive::seek(const s64 pos_) {
    _pos = pos_;
}

s64 MemoryBaseArchive::tell() const noexcept {
    return _pos;
}
