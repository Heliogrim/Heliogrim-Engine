#pragma once

#include <Engine.Common/Guid.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class PackageUrl final :
		public IUrlComp<PackageUrl> {
	public:
		using this_type = PackageUrl;

	public:
		constexpr PackageUrl() noexcept = default;

		explicit constexpr PackageUrl(mref<Guid> guid_) noexcept :
			_guid(std::move(guid_)) {}

		constexpr PackageUrl(cref<this_type>) noexcept = default;

		constexpr PackageUrl(mref<this_type>) noexcept = default;

		constexpr ~PackageUrl() noexcept = default;

	public:
		constexpr ref<PackageUrl> operator=(cref<PackageUrl> other_) noexcept = default;

		constexpr ref<PackageUrl> operator=(mref<PackageUrl> other_) noexcept = default;

	private:
		Guid _guid;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return PackageScheme;
		}

		[[nodiscard]] constexpr Guid guid() const noexcept {
			return _guid;
		}

		[[nodiscard]] String encode() const;
	};
}
