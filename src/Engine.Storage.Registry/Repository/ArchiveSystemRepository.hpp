#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Resource.Archive/ArchiveGuid.hpp>

#include "../IStorageRepository.hpp"

namespace hg::engine::storage::system {
	class ArchiveStorage;
	class PackageSystemProvider;
};

namespace hg::engine::storage::system {
	class ArchiveSystemRepository final :
		public IStorageRepository {
	public:
		friend class ::hg::engine::storage::system::PackageSystemProvider;

	public:
		using this_type = IStorageRepository;

	protected:
		explicit ArchiveSystemRepository(_In_ ref<PackageSystemProvider> provider_) noexcept;

	public:
		~ArchiveSystemRepository() override;

	private:
		nmpt<PackageSystemProvider> _spp;
		DenseMap<resource::ArchiveGuid, Arci<ArchiveStorage>> _storages;

	public:
		[[nodiscard]] std::span<const nmpt<const IStorageProvider>> getProviders() const noexcept override;

	public:
		[[nodiscard]] std::span<const UrlScheme> getUrlScheme() const noexcept override;

		[[nodiscard]] std::span<const u64> getRegisteredUrlScheme() const noexcept override;

	public:
		[[nodiscard]] Arci<IStorage> createStorage(mref<StorageDescriptor> descriptor_) override;

		[[nodiscard]] bool hasStorage(cref<Url> url_) const override;

		[[nodiscard]] Arci<IStorage> getStorageByUrl(cref<Url> url_) const override;

		void findReferrerStorages(cref<Arci<IStorage>> ref_, ref<Vector<Arci<IStorage>>> collector_) const override;

		[[nodiscard]] bool removeStorage(mref<nmpt<IStorage>> storage_) override;
	};
}
