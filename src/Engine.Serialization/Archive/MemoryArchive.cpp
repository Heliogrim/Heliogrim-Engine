#include "MemoryArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

MemoryArchive::MemoryArchive() :
    Archive(),
    _pos(0i64) {}

string_view MemoryArchive::getArchiveName() const noexcept {
    return "MemoryArchive"sv;
}

void MemoryArchive::seek(const s64 pos_) {
    _pos = pos_;
}

s64 MemoryArchive::tell() const noexcept {
    return _pos;
}
