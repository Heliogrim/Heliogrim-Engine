#include "PackageLinker.hpp"

#include <Engine.Serialization/Archive/SourceReadWriteArchive.hpp>
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

void PackageLinker::restoreLinks() {

    if (not _package || _package->header().indexSize <= 0) {
        return;
    }

    PackageIndexEntry index {};

    const streamoff begin = _package->header().indexOffset;
    const streamoff end = begin + _package->header().indexSize;
    streampos pos = begin;

    auto* const source = _package->_source.get();

    while (pos < end) {

        streamsize readBytes {};
        source->get(pos, sizeof(PackageIndexEntry), &index, readBytes);

        /**/

        if (index.offset < sizeof(PackageHeader) || index.offset >= begin) {
            __debugbreak();
            throw _STD runtime_error("Invalid offset stored at `PackageIndexEntry`.");
        }

        if ((index.offset + index.size) > begin) {
            __debugbreak();
            throw _STD runtime_error("Invalid size stored at `PackageIndexEntry`.");
        }

        /**/

        ArchiveHeader header {};
        source->get(index.offset, sizeof(ArchiveHeader), &header, readBytes);

        /**/

        _links.push_back(
            LinkedArchive {
                _STD move(header),
                _STD move(index)
            }
        );

        /**/

        pos += sizeof(PackageIndexEntry);
    }
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
        _package->_source->write(nextOffset, chunkSize, buffer, wrote);

        done += chunkSize;
    }

    /**/

    _links.push_back(
        LinkedArchive {
            _STD move(header_),
            PackageIndexEntry {
                static_cast<u64>(nextOffset),
                static_cast<u64>(archiveSize)
            }
        }
    );

    return false;
}

bool PackageLinker::store(std::initializer_list<std::pair<ArchiveHeader, uptr<serialization::Archive>>> archives_) {

    bool succeeded = false;
    for (auto&& pair : archives_) {
        //succeeded = store(_STD move(pair.first), _STD move(pair.second)) && succeeded;
    }

    return succeeded;
}

uptr<engine::serialization::SourceReadonlyArchive> PackageLinker::load(const Guid archiveGuid_) const noexcept {

    const auto where = _STD ranges::find(
        _links,
        archiveGuid_,
        [](cref<LinkedArchive> linked_) {
            return linked_.header.guid;
        }
    );

    if (where == _links.end()) {
        return nullptr;
    }

    const auto index = where->index;
    return make_uptr<serialization::SourceReadonlyArchive>(
        smr<Source>(_package->_source),
        static_cast<streamoff>(index.offset),
        static_cast<streamsize>(index.size)
    );
}

bool PackageLinker::containsArchive(const Guid archiveGuid_) const noexcept {
    return false;
}

u64 PackageLinker::getArchiveCount() const noexcept {
    return _links.size();
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

uptr<PackageLinker::readonly_iter_archive> PackageLinker::operator[](const_iterator_type where_) const noexcept {

    const auto index = where_->index;

    return make_uptr<readonly_iter_archive>(
        smr<Source>(_package->_source),
        static_cast<streamoff>(index.offset),
        static_cast<streamsize>(index.size)
    );
}

uptr<PackageLinker::iter_archive> PackageLinker::operator[](iterator_type where_) const noexcept {

    const auto index = where_->index;

    return make_uptr<serialization::SourceReadWriteArchive>(
        smr<Source>(_package->_source),
        static_cast<streamoff>(index.offset),
        static_cast<streamsize>(index.size)
    );
}
