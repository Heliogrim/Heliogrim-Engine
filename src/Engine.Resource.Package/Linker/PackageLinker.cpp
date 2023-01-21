#include "PackageLinker.hpp"

#include <Engine.Serialization/Archive/SourceBaseArchive.hpp>
#include <Engine.Serialization/Archive/SourceReadonlyArchive.hpp>

#include "LinkedArchiveIterator.hpp"

using namespace ember::engine::resource;
using namespace ember;

PackageLinker::PackageLinker(const non_owning_rptr<Package> package_) :
    _package(package_) {}

PackageLinker::~PackageLinker() = default;

const non_owning_rptr<const Package> PackageLinker::getPackage() const noexcept {
    return _package;
}

bool PackageLinker::canLoad() const noexcept {
    return false;
}

bool PackageLinker::canStore() const noexcept {
    return false;
}

bool PackageLinker::store(mref<ArchiveHeader> header_, mref<uptr<serialization::Archive>> archive_) {

    constexpr streamoff baseOffset = sizeof(PackageHeader);
    streamoff nextOffset = { baseOffset };

    if (not _links.empty()) {
        const auto lastIndex = _links.back().index;
        nextOffset = lastIndex.offset + lastIndex.size;
    }

    /**/

    const auto archive = _STD move(archive_);
    const auto archiveSize = archive->totalSize();

    archive->seek(0);

    u8 buffer[1024];
    streampos done = 0;
    while (not archive->isEOA()) {

        const streamsize left = archiveSize - done;
        const streamsize chunkSize = left > 1024 ? 1024 : left;

        // Warning: This is not intended to be used this way
        archive->serializeBytes(buffer, chunkSize, serialization::ArchiveStreamMode::eOut);

        [[maybe_unused]] streamsize wrote {};
        [[maybe_unused]] auto succeeded = _package->_source->write(nextOffset, chunkSize, buffer, wrote);

        done += chunkSize;
    }

    /**/

    _links.push_back(LinkedArchive {
        _STD move(header_),
        PackageIndexEntry {
            static_cast<u64>(nextOffset),
            static_cast<u64>(archiveSize)
        }
    });

    return false;
}

bool PackageLinker::store(std::initializer_list<std::pair<ArchiveHeader, uptr<serialization::Archive>>> archives_) {
    return false;
}

uptr<engine::serialization::SourceReadonlyArchive> PackageLinker::load(const Guid archiveGuid_) const noexcept {
    return nullptr;
}

bool PackageLinker::containsArchive(const Guid archiveGuid_) const noexcept {
    return false;
}

u64 PackageLinker::getArchiveCount() const noexcept {
    return 0ui64;
}

s64 PackageLinker::getArchiveSize() const noexcept {
    if (not _package) {
        return -1i64;
    }

    if (_links.empty()) {
        return 0i64;
    }

    const auto firstByte = _links.front().index.offset;
    const auto lastByte = _links.back().index.offset + _links.back().index.size;

    return lastByte - firstByte;
}

void PackageLinker::remove(const Guid archiveGuid_) {}

PackageLinker::const_iterator_type PackageLinker::begin() const noexcept {
    return const_iterator_type { this };
}

PackageLinker::iterator_type PackageLinker::begin() noexcept {
    return iterator_type { this };
}

PackageLinker::const_iterator_type PackageLinker::cbegin() const noexcept {
    return const_iterator_type { this };
}

PackageLinker::const_iterator_type PackageLinker::end() const noexcept {
    return const_iterator_type { this, _links.size() };
}

PackageLinker::iterator_type PackageLinker::end() noexcept {
    return iterator_type { this, _links.size() };
}

PackageLinker::const_iterator_type PackageLinker::cend() const noexcept {
    return const_iterator_type { this, _links.size() };
}

uptr<engine::serialization::SourceReadonlyArchive> PackageLinker::operator[
](const_iterator_type where_) const noexcept {
    return nullptr;
}

uptr<engine::serialization::SourceBaseArchive> PackageLinker::operator[](iterator_type where_) const noexcept {
    return nullptr;
}

bool PackageLinker::read() {
    return false;
}

bool PackageLinker::write() {
    return false;
}

void PackageLinker::readPackageIndex() {}

void PackageLinker::writePackageIndex() {}
