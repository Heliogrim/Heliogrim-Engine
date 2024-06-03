#pragma once

#include <Engine.Common/Guid.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class ArchiveUrl final :
		public IUrlComp<ArchiveUrl> {
	public:
		using this_type = ArchiveUrl;

	public:
		constexpr ArchiveUrl() noexcept = default;

		explicit constexpr ArchiveUrl(mref<Guid> guid_) noexcept :
			_guid(std::move(guid_)) {}

		constexpr ArchiveUrl(cref<this_type> other_) noexcept = default;

		constexpr ArchiveUrl(mref<this_type> other_) noexcept = default;

		constexpr ~ArchiveUrl() noexcept = default;

	private:
		Guid _guid;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return ArchiveScheme;
		}

		[[nodiscard]] constexpr Guid guid() const noexcept {
			return _guid;
		}

		[[nodiscard]] String encode() const;
	};
}
