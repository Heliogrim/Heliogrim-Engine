#include "Archive.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>

using namespace hg::engine::serialization;
using namespace hg;

Archive::Archive() noexcept :
    _error(),
    _filter(nullptr),
    _compression(nullptr),
    _version() {}

bool Archive::hasError() const noexcept {
    return _error;
}

void Archive::setError() noexcept {
    _error = true;
}

void Archive::clearError() noexcept {
    _error = false;
}

void Archive::seek(const s64 pos_) {}

s64 Archive::tell() const noexcept {
    return -1i64;
}

s64 Archive::totalSize() const noexcept {
    return -1i64;
}

bool Archive::isEOA() const noexcept {
    return (totalSize() == -1i64) || tell() >= totalSize();
}

void Archive::flush() {}

bool Archive::close() {
    return true;
}

bool Archive::hasFilter() const noexcept {
    return _filter != nullptr;
}

const ptr<ArchiveFilter> Archive::getArchiveFilter() const noexcept {
    return _filter;
}

void Archive::setFilter(const ptr<ArchiveFilter> filter_) {
    _filter = filter_;
}

bool Archive::hasCompression() const noexcept {
    return _compression != nullptr;
}

const ptr<ArchiveCompression> Archive::getArchiveCompression() const noexcept {
    return _compression;
}

void Archive::setCompression(const ptr<ArchiveCompression> compression_) {
    _compression = compression_;
}

ArchiveVersion Archive::getArchiveVersion() const noexcept {
    return _version;
}

void Archive::setArchiveVersion(cref<ArchiveVersion> version_) noexcept {
    _version = version_;
}

void Archive::serializeBytesSwapped(ref<u16> value_, const ArchiveStreamMode mode_) {
    throw NotImplementedException();
}

void Archive::serializeBytesSwapped(ref<u32> value_, const ArchiveStreamMode mode_) {
    throw NotImplementedException();
}

void Archive::serializeBytesSwapped(ref<u64> value_, const ArchiveStreamMode mode_) {
    throw NotImplementedException();
}

void Archive::serializeBytesSwapped(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) {
    throw NotImplementedException();
}

ref<Archive::this_type> Archive::operator<<(const ptr<ClassMetaBase> object_) {
    throw NotImplementedException();
    return *this;
}
