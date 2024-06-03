#pragma once

#include "IStorageDescriptor.hpp"

namespace hg::engine::storage {
	class PackageStorageDescriptor final :
		public IStorageDescriptor<PackageStorageDescriptor> {
	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return PackageScheme;
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return true;
		}
	};
}
