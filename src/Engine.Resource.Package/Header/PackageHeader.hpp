#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Array.hpp>

#include "../Attribute/PackageCompression.hpp"
#include "../Attribute/PackageEndianness.hpp"
#include "../Attribute/PackageVersion.hpp"

namespace hg::engine::resource {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageHeader {
		// 0x00
		Array<_::byte, 6> magicBytes;
		u8 magicVersion;
		PackageEndianness endianness;
		// 0x08
		PackageVersion version;

		// 0x10
		Array<_::byte, 16> guid;// ~ ::hg::Guid is not trivial

		// 0x20
		PackageCompression compression;
		u16 __reserved__;
		// 0x24
		u64 packageSize;
		u64 inflatedSize;
		// 0x34
		u64 indexOffset;
		u64 indexSize;
		// 0x44
		u64 metaOffset;
		u64 metaSize;
		// 0x54
		u64 compDataOffset;
		u64 compDataSize;
		// 0x64
	};
	#pragma pack(pop)

	static_assert(std::is_trivial_v<PackageHeader>);
	static_assert(sizeof(PackageHeader) == 0x64);
}
