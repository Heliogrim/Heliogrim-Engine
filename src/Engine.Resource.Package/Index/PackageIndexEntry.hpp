#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::resource {
	#pragma pack(push)
	#pragma pack(1)
	struct PackageIndexEntry {
		streamoff offset;
		streamsize size;
	};
	#pragma pack(pop)

	static_assert(std::is_trivial_v<PackageIndexEntry>);
	static_assert(sizeof(PackageIndexEntry) == 0x10);
}
