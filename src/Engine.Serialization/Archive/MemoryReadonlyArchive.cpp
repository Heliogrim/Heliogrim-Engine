#include "MemoryReadonlyArchive.hpp"

#include <format>

using namespace hg::engine::serialization;
using namespace hg;

MemoryReadonlyArchive::MemoryReadonlyArchive(ref<_STD span<u8, _STD dynamic_extent>> bytes_) :
    MemoryBaseArchive(),
    _bytes(bytes_),
    _limit(-1) {}

MemoryReadonlyArchive::MemoryReadonlyArchive(ref<_STD span<u8, _STD dynamic_extent>> bytes_, const s64 limit_) :
    MemoryBaseArchive(),
    _bytes(bytes_),
    _limit(limit_) {}

Url MemoryReadonlyArchive::getArchiveUrl() const noexcept {
    const string path {
        _STD format(
            "{:#016x}:{:#016x}",
            reinterpret_cast<u64>(_bytes.data()),
            reinterpret_cast<u64>(_bytes.data() + totalSize())
        )
    };
    return Url { "mem"sv, path };
}

s64 MemoryReadonlyArchive::totalSize() const noexcept {
    return _limit >= 0 ? MIN(_limit, static_cast<s64>(_bytes.size())) : static_cast<s64>(_bytes.size());
}

bool MemoryReadonlyArchive::hasSizeLimit() const noexcept {
    return _limit >= 0i64;
}

s64 MemoryReadonlyArchive::sizeLimit() const noexcept {
    return _limit;
}

void MemoryReadonlyArchive::setSizeLimit(const s64 limit_) {
    _limit = limit_;
}

void MemoryReadonlyArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError() || mode_ == ArchiveStreamMode::eIn) [[unlikely]]
    {
        return;
    }

    if (_pos + size_ <= _bytes.size()) {

        _STD memcpy(value_, &_bytes[_pos], size_);
        _pos += static_cast<s64>(size_);

    } else [[unlikely]]
    {
        setError();
    }
}
