#include "MemoryReadWriteArchive.hpp"

#include <format>

using namespace hg::engine::serialization;
using namespace hg;

MemoryReadWriteArchive::MemoryReadWriteArchive(ref<AutoArray<u8>> bytes_) :
    MemoryBaseArchive(),
    _bytes(bytes_),
    _limit(-1) {}

MemoryReadWriteArchive::MemoryReadWriteArchive(ref<AutoArray<u8>> bytes_, const s64 limit_) :
    MemoryBaseArchive(),
    _bytes(bytes_),
    _limit(limit_) {}

Url MemoryReadWriteArchive::getArchiveUrl() const noexcept {
    const string path {
        _STD format(
            "{:#016x}:{:#016x}",
            reinterpret_cast<u64>(_bytes.data()),
            reinterpret_cast<u64>(_bytes.data() + totalSize())
        )
    };
    return Url { "mem"sv, path };
}

s64 MemoryReadWriteArchive::totalSize() const noexcept {
    return _limit >= 0i64 ? MIN(_limit, static_cast<s64>(_bytes.size())) : _bytes.size();
}

bool MemoryReadWriteArchive::hasSizeLimit() const noexcept {
    return _limit >= 0;
}

s64 MemoryReadWriteArchive::sizeLimit() const noexcept {
    return _limit;
}

void MemoryReadWriteArchive::setSizeLimit(const s64 limit_) {
    _limit = limit_;
}

void MemoryReadWriteArchive::serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {

    if (size_ <= 0 || hasError()) [[unlikely]]
    {
        return;
    }

    if (mode_ == ArchiveStreamMode::eIn) {

        const s64 overflow { _pos + static_cast<s64>(size_) - static_cast<s64>(_bytes.size()) };
        if (overflow > 0) {

            // TODO: Add possibility to expand AutoArray<T> with uninitialized memory sequence, cause we sometimes might just override the new memory
            const auto nextSize { _bytes.size() + overflow };
            _bytes.resize(nextSize);
        }

        _STD memcpy(&_bytes[_pos], value_, size_);
        _pos += static_cast<s64>(size_);

    } else /* if (mode_ == ArchiveStreamMode::eOut) */ {

        if (_pos + size_ <= _bytes.size()) {

            _STD memcpy(value_, &_bytes[_pos], size_);
            _pos += static_cast<s64>(size_);

        } else [[unlikely]]
        {
            setError();
        }

    }
}
