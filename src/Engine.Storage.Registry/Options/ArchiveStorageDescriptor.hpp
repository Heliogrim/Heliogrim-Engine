#pragma once

#include "IStorageDescriptor.hpp"
#include "../IStorage.hpp"
#include "../Url/ArchiveUrl.hpp"

namespace hg::engine::storage {
	class ArchiveStorageDescriptor final :
		public IStorageDescriptor<ArchiveStorageDescriptor> {
	public:
		using this_type = ArchiveStorageDescriptor;

	public:
		ArchiveStorageDescriptor() = delete;

		explicit constexpr ArchiveStorageDescriptor(
			mref<ArchiveUrl> archiveUrl_,
			mref<Arci<IStorage>> archiveBacking_
		) noexcept :
			_archiveUrl(std::move(archiveUrl_)),
			_archiveBacking(std::move(archiveBacking_)) {}

		constexpr ArchiveStorageDescriptor(cref<this_type>) = delete;

		constexpr ArchiveStorageDescriptor(mref<this_type>) noexcept = default;

		constexpr ~ArchiveStorageDescriptor() noexcept = default;

	private:
		ArchiveUrl _archiveUrl;
		Arci<IStorage> _archiveBacking;

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const noexcept {
			return ArchiveScheme;
		}

		[[nodiscard]] constexpr resource::ArchiveGuid archiveGuid() const noexcept {
			return _archiveUrl.guid();
		}

		[[nodiscard]] Arci<IStorage> archiveBacking() const noexcept {
			return _archiveBacking;
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return (_archiveUrl.guid() != resource::ArchiveGuid {}) && _archiveBacking != nullptr;
		}
	};
}
