#include "BufferArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

BufferArchive::BufferArchive() :
    MemoryReadWriteArchive(*static_cast<ptr<AutoArray<u8>>>(this)),
    AutoArray<u8>() {}

string_view BufferArchive::getArchiveName() const noexcept {
    return "BufferArchive"sv;
}
