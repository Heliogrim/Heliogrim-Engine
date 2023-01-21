#include "Package.hpp"

#include "../Linker/PackageLinker.hpp"

#include "MagicBytes.hpp"

#include <Engine.Serialization/Archive/BufferArchive.hpp>

using namespace ember::engine::resource;
using namespace ember;

Package::Package(
    mref<uptr<res::Source>> source_,
    mref<PackageHeader> header_,
    mref<PackageFooter> footer_
) :
    _header(_STD move(header_)),
    _footer(_STD move(footer_)),
    _source(_STD move(source_)) {
    _linker = uptr<PackageLinker>(new PackageLinker(this));
}

Package::~Package() {}

ref<PackageHeader> Package::header() noexcept {
    return _header;
}

ref<PackageFooter> Package::footer() noexcept {
    return _footer;
}

const non_owning_rptr<PackageLinker> Package::getLinker() const noexcept {
    return _linker.get();
}

void Package::unsafeReleaseSource(ref<uptr<res::Source>> dst_) {
    dst_ = _STD move(_source);
}

void Package::unsafeWrite() {

    constexpr streamoff headOff = sizeof(PackageHeader);

    const auto* const linker = _linker.get();
    const streamoff linkSize = linker->getArchiveSize();

    // TODO: Check whether linker is stalled

    // Write Index and Meta
    unsafeWriteIndex(headOff + linkSize);
    unsafeWriteMeta(headOff + linkSize + _header.indexSize);

    /**/

    // Update Metrics
    _header.inflatedSize = linkSize + _header.indexSize + _header.metaSize;

    // TODO: 1. Calc CRC-32 for footer signature
    // TODO: 2. Compress

    _header.packageSize = /* Compressed Size ... */ _header.inflatedSize;

    // Rewrite Header
    unsafeWriteHeader();

    // Write Footer
    unsafeWriteFooter(headOff + linkSize + _header.indexSize + _header.metaSize);
}

void Package::unsafeWriteHeader() {

    serialization::BufferArchive archive {};
    archive.reserve(sizeof(PackageHeader));

    archive.seek(0);

    archive.serializeBytes(const_cast<ptr<u8>>(PackageMagicBytes), sizeof(PackageMagicBytes),
        serialization::ArchiveStreamMode::eIn);
    archive << PackageMagicVersion[0];
    archive << PackageEndianness::eBigEndian;

    archive << _header.version;
    archive.serializeBytes(_header.guid, sizeof(_header.guid), serialization::ArchiveStreamMode::eIn);
    archive << _header.compression;

    archive << 0ui16;// Padding

    archive << _header.packageSize;
    archive << _header.inflatedSize;

    archive << _header.indexOffset;
    archive << _header.indexSize;
    archive << _header.metaOffset;
    archive << _header.metaSize;
    archive << _header.compDataOffset;
    archive << _header.compDataSize;

    //

    [[maybe_unused]] streamsize wrote = 0;
    [[maybe_unused]] const auto succeeded = _source->write(0, archive.size(), archive.data(), wrote);
}

void Package::unsafeWriteIndex(streamoff pos_) {

    const auto* const linker = _linker.get();
    if (not linker) {
        return;
    }

    const auto entries = linker->_links.size();
    const auto indexSize = entries * sizeof(PackageIndexEntry);

    serialization::BufferArchive archive {};
    archive.reserve(indexSize);

    archive.seek(0);

    for (const auto& linkerEntry : linker->_links) {
        archive << linkerEntry.index.offset;
        archive << linkerEntry.index.size;
    }

    //

    [[maybe_unused]] streamsize wrote = 0;
    [[maybe_unused]] const auto succeeded = _source->write(pos_, archive.size(), archive.data(), wrote);

    //

    _header.indexOffset = pos_;
    _header.indexSize = archive.size();
}

void Package::unsafeWriteMeta(streamoff pos_) {

    /*
    Archive archive {};
     */

    /*
    [[maybe_unused]] streamsize wrote = 0;
    [[maybe_unused]] const auto succeeded = _source->write(pos_, 0, nullptr, wrote);
     */

    //

    _header.metaOffset = pos_;
    _header.metaSize = /* archive.size() */ 0;
}

void Package::unsafeWriteFooter(streamoff pos_) {

    serialization::BufferArchive archive {};
    archive.reserve(sizeof(PackageFooter));

    archive.seek(0);

    u8 crc32[8] { 0ui8 };
    archive.serializeBytes(crc32, sizeof(crc32), serialization::ArchiveStreamMode::eIn);

    u8 reserved[8] { 0ui8 };
    archive.serializeBytes(reserved, sizeof(reserved), serialization::ArchiveStreamMode::eIn);

    archive.serializeBytes(const_cast<ptr<u8>>(PackageMagicBytes), sizeof(PackageMagicBytes),
        serialization::ArchiveStreamMode::eIn);
    archive << PackageMagicVersion[0];
    archive << PackageEndianness::eBigEndian;

    //

    [[maybe_unused]] streamsize wrote = 0;
    [[maybe_unused]] const auto succeeded = _source->write(pos_, archive.size(), archive.data(), wrote);
}
