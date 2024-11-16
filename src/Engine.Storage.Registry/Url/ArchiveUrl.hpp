#pragma once

#include <Engine.Resource.Archive/ArchiveGuid.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class alignas(alignof(void*)) ArchiveUrl final :
		public IUrlComp<ArchiveUrl> {
	public:
		using this_type = ArchiveUrl;

	public:
		constexpr ArchiveUrl() noexcept = default;

		explicit constexpr ArchiveUrl(cref<resource::ArchiveGuid> guid_) noexcept :
			_guid(guid_) {}

		constexpr ArchiveUrl(cref<this_type> other_) noexcept = default;

		constexpr ArchiveUrl(mref<this_type> other_) noexcept = default;

		constexpr ~ArchiveUrl() noexcept = default;

	public:
		constexpr ref<ArchiveUrl> operator=(cref<ArchiveUrl> other_) noexcept = default;

		constexpr ref<ArchiveUrl> operator=(mref<ArchiveUrl> other_) noexcept = default;

	private:
		resource::ArchiveGuid _guid {};

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return ArchiveScheme;
		}

		[[nodiscard]] constexpr resource::ArchiveGuid guid() const noexcept {
			return _guid;
		}

		[[nodiscard]] String encode() const;
	};
}
