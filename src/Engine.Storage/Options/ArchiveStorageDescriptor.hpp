#pragma once

#include "IStorageDescriptor.hpp"
#include "../Url/ArchiveUrl.hpp"

namespace hg::engine::storage {
	class ArchiveStorageDescriptor final :
		public IStorageDescriptor<ArchiveStorageDescriptor> {
	public:
		using this_type = ArchiveStorageDescriptor;

	public:
		constexpr ArchiveStorageDescriptor() noexcept = default;

		explicit constexpr ArchiveStorageDescriptor(mref<ArchiveUrl> archiveUrl_) noexcept :
			_archiveUrl(std::move(archiveUrl_)) {}

		constexpr ArchiveStorageDescriptor(cref<this_type>) = delete;

		constexpr ArchiveStorageDescriptor(mref<this_type>) noexcept = default;

		constexpr ~ArchiveStorageDescriptor() noexcept = default;

	private:
		ArchiveUrl _archiveUrl;

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return ArchiveScheme;
		}

		[[nodiscard]] constexpr Guid archiveGuid() const noexcept {
			return _archiveUrl.guid();
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return true;
		}
	};
}
