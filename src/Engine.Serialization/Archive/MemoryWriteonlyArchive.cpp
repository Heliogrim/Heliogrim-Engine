#include "MemoryWriteonlyArchive.hpp"

#include <format>

using namespace hg::engine::serialization;
using namespace hg;

MemoryWriteonlyArchive::MemoryWriteonlyArchive(ref<AutoArray<u8>> bytes_) :
    MemoryBaseArchive(),
    _bytes(bytes_) {}

fs::Url MemoryWriteonlyArchive::getArchiveUrl() const noexcept {
    return fs::Url {
        "mem"sv, fs::Path {
            _STD format(
                "{:#016x}:{:#016x}",
                reinterpret_cast<u64>(_bytes.data()),
                reinterpret_cast<u64>(_bytes.data() + totalSize())
            )
        }
    };
}

s64 MemoryWriteonlyArchive::totalSize() const noexcept {
    return static_cast<s64>(_bytes.size());
}

void MemoryWriteonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eOut) [[unlikely]]
    {
        return;
    }

    const s64 overflow { _pos + static_cast<s64>(size_) - static_cast<s64>(_bytes.size()) };
    if (overflow > 0) {

        // TODO: Add possibility to expand AutoArray<T> with uninitialized memory sequence, cause we sometimes might just override the new memory
        const auto nextSize { _bytes.size() + overflow };
        _bytes.resize(nextSize);

    }

    _STD memcpy(&_bytes[_pos], value_, size_);
    _pos += static_cast<s64>(size_);
}
