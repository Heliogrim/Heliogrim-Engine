#pragma once

#include <type_traits>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Array.hpp>

#include "../Attribute/PackageEndianness.hpp"

namespace hg::engine::resource {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageFooter {
		// 0x00
		Array<_::byte, 8> crc32;
		Array<_::byte, 8> __reserved__;
		// 0x10
		Array<_::byte, 6> magicBytes;
		u8 magicVersion;
		PackageEndianness endianess;
		// 0x18
	};
	#pragma pack(pop)

	static_assert(std::is_trivial_v<PackageFooter>);
	static_assert(sizeof(PackageFooter) == 0x18);
}
