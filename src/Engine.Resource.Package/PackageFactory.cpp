#include "PackageFactory.hpp"

#include <cassert>

#include <Engine.Common/Make.hpp>

#include "Package/MagicBytes.hpp"
#include "Package/PackageGuid.hpp"
#include "Package/PackageHeader.hpp"

using namespace hg::engine::resource;
using namespace hg;

uptr<Package> PackageFactory::createEmptyPackage(mref<uptr<Source>> source_) {

    PackageGuid guid {};
    GuidGenerate(guid);

    return make_uptr<Package>(
        std::move(source_),
        PackageHeader {
            .magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B }/*PackageMagicBytes */,
            .magicVersion = PackageMagicVersion[0],
            .endianness = PackageEndianness::eBigEndian,
            .version = 0x1,
            .guidData = std::move(guid.data),
            .compression = PackageCompression::eNone
        },
        PackageFooter {
            .crc32 = {},
            .magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B }/*PackageMagicBytes */,
            .magicVersion = PackageMagicVersion[0],
            .endianess = PackageEndianness::eBigEndian
        }
    );
}

uptr<Package> PackageFactory::createFromSource(mref<uptr<Source>> source_) {

    PackageHeader storedHeader {};
    PackageFooter storedFooter {};

    streamsize readBytes { ~0 };
    auto succeeded = source_->get(0, sizeof(PackageHeader), reinterpret_cast<ptr<u8>>(&storedHeader), readBytes);

    if (storedHeader.magicVersion != PackageMagicVersion[0]) {
        throw std::runtime_error("");
    }

    if (memcmp(storedHeader.magicBytes, PackageMagicBytes, sizeof(PackageMagicBytes)) != 0) {
        throw std::runtime_error("");
    }

    /**/

    const streamoff offset = sizeof(PackageHeader) + storedHeader.packageSize
        /* Attention: Sometimes we are required to use inflated size. */;

    readBytes = ~0;
    succeeded = source_->get(offset, sizeof(PackageFooter), reinterpret_cast<ptr<u8>>(&storedFooter), readBytes);

    if (storedFooter.magicVersion != PackageMagicVersion[0]) {
        throw std::runtime_error("");
    }

    if (memcmp(storedFooter.magicBytes, PackageMagicBytes, sizeof(PackageMagicBytes)) != 0) {
        throw std::runtime_error("");
    }

    /**/

    // TODO: Validate Package creation ( maybe calc and check crc-32 of footer )

    /**/

    return make_uptr<Package>(
        std::move(source_),
        std::move(storedHeader),
        std::move(storedFooter)
    );
}
