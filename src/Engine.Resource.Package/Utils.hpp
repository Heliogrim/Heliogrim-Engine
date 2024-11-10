#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Header/PackageHeader.hpp"

namespace hg::engine::resource::package {
	[[nodiscard]] constexpr streamoff computeFooterOffset(_In_ ref<const PackageHeader> header_) noexcept {
		return static_cast<streamsize>(sizeof(PackageHeader) + header_.packageSize);
	}

	[[nodiscard]] constexpr streamsize computeContentSize(_In_ ref<const PackageHeader> header_) noexcept {
		if (header_.indexOffset == 0) { return 0LL; }
		return static_cast<streamsize>(header_.indexOffset - sizeof(PackageHeader));
	}
}
