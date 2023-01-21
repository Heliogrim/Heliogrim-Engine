#include "pch.h"
#include <Engine.Common/Make.hpp>
#include <Engine.Resource.Package/PackageFactory.hpp>
#include <Engine.Resource.Package/Linker/PackageLinker.hpp>
#include <Engine.Resource.Package/Package/MagicBytes.hpp>
#include <Engine.Resource.Package/Package/Package.hpp>
#include <Engine.Resource.Package/Package/PackageGuid.hpp>
#include <Engine.Resource/Source/BufferSource.hpp>
#include <Engine.Resource.Package/Linker/LinkedArchiveIterator.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/SourceReadonlyArchive.hpp>
#include <random>

using namespace ember::engine::resource;
using namespace ember::engine::res;
using namespace ember::engine::serialization;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace PackageModule {
    [[nodiscard]] _STD pair<BufferArchive, uptr<BufferSource>> makeTestSource() {

        auto helper = BufferArchive {};
        helper.resize(4096, 0ui8);

        auto* const ptr = helper.data();

        return _STD make_pair<>(
            _STD move(helper),
            make_uptr<BufferSource>(Buffer {
                4096,
                0,
                ptr
            })
        );
    }

    void generatePayload(ref<BufferArchive> archive_, const u64 size_ = 48ui64) {

        _STD random_device device {};
        _STD mt19937 mt { device() };

        for (u64 i = 0; i < size_; ++i) {
            archive_ << mt();
        }
    }

    TEST(Package, EmptyWrite) {

        // Attention: TBD: Check for system endianness

        auto [helper, source] = makeTestSource();

        Package package = PackageFactory::createEmptyPackage(_STD move(source));

        auto* const linker = package.getLinker();

        /**/

        const u8* const view = static_cast<const u8* const>(helper.data());
        const auto* const storedHeader = reinterpret_cast<const ptr<const PackageHeader>>(helper.data());

        for (size_t i = 0; i < 4096; ++i) {
            EXPECT_EQ(view[i], 0);
        }

        /**/

        PackageVersion version { 27859368927ui64 };
        PackageGuid guid { 278608376ui32, 11632ui16, 7635ui16, 36758197986519ui64 };

        /**/

        ref<PackageHeader> header = package.header();
        ref<PackageFooter> footer = package.footer();

        memcpy(header.magicBytes, PackageMagicBytes, sizeof(PackageMagicBytes));
        header.magicVersion = PackageMagicVersion[0];
        header.endianness = PackageEndianness::eBigEndian;

        header.version = version;
        memcpy(header.guid, guid.bytes, sizeof(PackageGuid));

        header.compression = PackageCompression::eNone;
        header.packageSize = 0ui64;
        header.inflatedSize = 0ui64;
        header.indexOffset = 0ui64;
        header.indexSize = 0ui64;
        header.metaOffset = 0ui64;
        header.metaSize = 0ui64;
        header.compDataOffset = 0ui64;
        header.compDataSize = 0ui64;

        // Write Package data
        package.unsafeWrite();// ??

        /**/

        // Check Magic Bytes
        {
            for (auto i = 0; i < sizeof(PackageMagicBytes); ++i) {
                EXPECT_EQ(storedHeader->magicBytes[i], PackageMagicBytes[i]);
            }

            EXPECT_EQ(storedHeader->magicVersion, PackageMagicVersion[0]);
            EXPECT_EQ(storedHeader->endianness, PackageEndianness::eBigEndian);
        }

        // Check Package Version
        {
            EXPECT_EQ(storedHeader->version, version);
        }

        // Check Package Guid
        {
            for (auto i = 0; i < sizeof(PackageGuid); ++i) {
                EXPECT_EQ(storedHeader->guid[i], guid.bytes[i]);
            }
        }

        // Check Package Compression
        {
            EXPECT_EQ(storedHeader->compression, PackageCompression::eNone);
        }

        // Check Package/Inflate Size
        {
            EXPECT_EQ(storedHeader->packageSize, 0);
            EXPECT_EQ(storedHeader->inflatedSize, 0);
        }

        // Check Index, Meta and Compression Segment
        {
            EXPECT_EQ(storedHeader->indexOffset, sizeof(PackageHeader));
            EXPECT_EQ(storedHeader->indexSize, 0);

            EXPECT_EQ(storedHeader->metaOffset, sizeof(PackageHeader));
            EXPECT_EQ(storedHeader->metaSize, 0);

            EXPECT_EQ(storedHeader->compDataOffset, 0);
            EXPECT_EQ(storedHeader->compDataSize, 0);
        }
    }

    TEST(Package, EmptyReadWrite) {
        // Attention: TBD: Check for system endianness

        auto [helper, source] = makeTestSource();

        Package package = PackageFactory::createEmptyPackage(_STD move(source));

        auto* const linker = package.getLinker();

        /**/

        PackageVersion version { 27859368927ui64 };
        PackageGuid guid { 278608376ui32, 11632ui16, 7635ui16, 36758197986519ui64 };

        /**/

        ref<PackageHeader> header = package.header();
        ref<PackageFooter> footer = package.footer();

        memcpy(header.magicBytes, PackageMagicBytes, sizeof(PackageMagicBytes));
        header.magicVersion = PackageMagicVersion[0];
        header.endianness = PackageEndianness::eBigEndian;

        header.version = version;
        memcpy(header.guid, guid.bytes, sizeof(PackageGuid));

        header.compression = PackageCompression::eNone;
        header.packageSize = 0ui64;
        header.inflatedSize = 0ui64;
        header.indexOffset = 0ui64;
        header.indexSize = 0ui64;
        header.metaOffset = 0ui64;
        header.metaSize = 0ui64;
        header.compDataOffset = 0ui64;
        header.compDataSize = 0ui64;

        // Write Package data
        package.unsafeWrite();// ??

        /**/

        uptr<Source> dstSource = make_uptr<BufferSource>(Buffer {
            4096ui64,
            0ui64,
            helper.data()
        });

        Package dst = PackageFactory::createFromSource(_STD move(dstSource));

        const auto* const storedHeader = &dst.header();

        /**/

        // Check Magic Bytes
        {
            for (auto i = 0; i < sizeof(PackageMagicBytes); ++i) {
                EXPECT_EQ(storedHeader->magicBytes[i], PackageMagicBytes[i]);
            }

            EXPECT_EQ(storedHeader->magicVersion, PackageMagicVersion[0]);
            EXPECT_EQ(storedHeader->endianness, PackageEndianness::eBigEndian);
        }

        // Check Package Version
        {
            EXPECT_EQ(storedHeader->version, version);
        }

        // Check Package Guid
        {
            for (auto i = 0; i < sizeof(PackageGuid); ++i) {
                EXPECT_EQ(storedHeader->guid[i], guid.bytes[i]);
            }
        }

        // Check Package Compression
        {
            EXPECT_EQ(storedHeader->compression, PackageCompression::eNone);
        }

        // Check Package/Inflate Size
        {
            EXPECT_EQ(storedHeader->packageSize, 0);
            EXPECT_EQ(storedHeader->inflatedSize, 0);
        }

        // Check Index, Meta and Compression Segment
        {
            EXPECT_EQ(storedHeader->indexOffset, sizeof(PackageHeader));
            EXPECT_EQ(storedHeader->indexSize, 0);

            EXPECT_EQ(storedHeader->metaOffset, sizeof(PackageHeader));
            EXPECT_EQ(storedHeader->metaSize, 0);

            EXPECT_EQ(storedHeader->compDataOffset, 0);
            EXPECT_EQ(storedHeader->compDataSize, 0);
        }
    }

    TEST(Package, MetaReadWrite) { }

    TEST(Package, SingleReadWrite) {
        // Attention: TBD: Check for system endianness

        auto [helper, source] = makeTestSource();

        Package package = PackageFactory::createEmptyPackage(_STD move(source));

        /**/

        // Create Payload
        Guid payloadGuid {};
        GuidGenerate(payloadGuid);

        auto payload = make_uptr<BufferArchive>();

        const auto valCount = 48ui64;
        generatePayload(*payload, valCount);

        BufferArchive replica {};
        replica.reserve(payload->size());
        replica.insert(replica.end(), payload->begin(), payload->end());

        auto* const srcLinker = package.getLinker();
        srcLinker->store(
            ArchiveHeader {
                ArchiveHeaderType::eRaw,
                payloadGuid
            },
            _STD move(payload)
        );

        // Write Package data

        // TODO: Where should the write call occure?
        // TODO: `package.write(...)` vs `linker.write(...)`

        package.unsafeWrite();// ??

        /**/

        uptr<Source> dstSource = make_uptr<BufferSource>(Buffer {
            4096ui64,
            0ui64,
            helper.data()
        });

        Package dst = PackageFactory::createFromSource(_STD move(dstSource));
        const auto* const dstLinker = package.getLinker();

        EXPECT_EQ(dstLinker->getArchiveCount(), 1);
        EXPECT_NE(dstLinker->begin(), dstLinker->end());

        auto archiveIter = dstLinker->begin();

        auto storedHeader = archiveIter.header();
        auto storedArchive = archiveIter.archive();

        EXPECT_EQ(storedHeader.type, ArchiveHeaderType::eRaw);
        EXPECT_EQ(storedHeader.guid, payloadGuid);

        EXPECT_NE(storedArchive, nullptr);

        // TODO: Check whether `memcmp(...)` might be possible with `storedArchive` and `replica`
        // TODO: Currently value iterated compare to prevent source loading issue cause there are not memory blocks at SourceArchive

        storedArchive->seek(0);
        replica.seek(0);
        for (size_t i = 0; i < valCount; ++i) {

            using value_type = _STD mt19937::result_type;

            value_type value {};
            (*storedArchive) >> value;

            value_type check {};
            (*storedArchive) >> check;

            EXPECT_EQ(value, check);
        }
    }

    TEST(Package, MultiReadWrite) { }
}
