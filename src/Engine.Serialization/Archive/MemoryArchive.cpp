#include "MemoryArchive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

MemoryBaseArchive::MemoryBaseArchive() :
    Archive(),
    _pos(0i64) {}

string_view MemoryBaseArchive::getArchiveName() const noexcept {
    return "MemoryArchive"sv;
}

void MemoryBaseArchive::seek(const s64 pos_) {
    _pos = pos_;
}

s64 MemoryBaseArchive::tell() const noexcept {
    return _pos;
}
