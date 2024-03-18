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

	u8 htonGuid[16] {};
	guid.hton(htonGuid);

	return make_uptr<Package>(
		std::move(source_),
		PackageHeader {
			.magicBytes = { 0x49, 0x4D, 0x50, 0x41, 0x43, 0x4B }/*PackageMagicBytes */,
			.magicVersion = PackageMagicVersion[0],
			.endianness = PackageEndianness::eBigEndian,
			.version = 0x1,
			.guid = {
				htonGuid[0], htonGuid[1], htonGuid[2], htonGuid[3],
				htonGuid[4], htonGuid[5], htonGuid[6], htonGuid[7],
				htonGuid[8], htonGuid[9], htonGuid[10], htonGuid[11],
				htonGuid[12], htonGuid[13], htonGuid[14], htonGuid[15]
			},
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
