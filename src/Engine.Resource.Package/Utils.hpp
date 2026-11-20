#pragma once

#include <Engine.Common/Sal.hpp>

#include "Header/PackageHeader.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource::package {
	[[nodiscard]] constexpr streamoff computeFooterOffset(_In_ ref<const PackageHeader> header_) noexcept {
		return static_cast<streamsize>(sizeof(PackageHeader) + header_.packageSize);
	}

	[[nodiscard]] constexpr streamsize computeContentSize(_In_ ref<const PackageHeader> header_) noexcept {
		if (header_.indexOffset == 0) { return 0LL; }
		return static_cast<streamsize>(header_.indexOffset - sizeof(PackageHeader));
	}
}
