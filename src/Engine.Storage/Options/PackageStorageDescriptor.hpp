#pragma once

#include "IStorageDescriptor.hpp"
#include "../IStorage.hpp"
#include "../Url/PackageUrl.hpp"

namespace hg::engine::storage {
	class PackageStorageDescriptor final :
		public IStorageDescriptor<PackageStorageDescriptor> {
	public:
		using this_type = PackageStorageDescriptor;

	public:
		PackageStorageDescriptor() = delete;

		explicit constexpr PackageStorageDescriptor(
			mref<PackageUrl> packageUrl_,
			mref<Arci<IStorage>> packageBacking_
		) noexcept :
			_packageUrl(std::move(packageUrl_)),
			_packageBacking(std::move(packageBacking_)) {}

		constexpr PackageStorageDescriptor(cref<this_type>) = delete;

		constexpr PackageStorageDescriptor(mref<this_type>) noexcept = default;

		constexpr ~PackageStorageDescriptor() noexcept = default;

	private:
		PackageUrl _packageUrl;
		Arci<IStorage> _packageBacking;

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return PackageScheme;
		}

		[[nodiscard]] constexpr Guid packageGuid() const noexcept {
			return _packageUrl.guid();
		}

		[[nodiscard]] Arci<IStorage> packageBacking() const noexcept {
			return _packageBacking;
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return (_packageUrl.guid() != Guid {}) && (_packageBacking != nullptr);
		}
	};
}
