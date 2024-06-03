#pragma once

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class PackageUrl final :
		public IUrlComp<PackageUrl> {
	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return PackageScheme;
		}
	};
}
