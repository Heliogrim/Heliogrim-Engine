#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "../IStorageRepository.hpp"

namespace hg::engine::storage::system {
	class PackageStorage;
	class PackageSystemProvider;
}

namespace hg::engine::storage::system {
	class PackageSystemRepository final :
		public IStorageRepository {
	public:
		friend class PackageSystemProvider;

	public:
		using this_type = IStorageRepository;

	protected:
		explicit PackageSystemRepository(_In_ ref<PackageSystemProvider> provider_) noexcept;

	public:
		~PackageSystemRepository() override;

	private:
		nmpt<PackageSystemProvider> _spp;
		DenseMap<Guid, Arci<PackageStorage>> _storages;

	public:
		[[nodiscard]] std::span<const nmpt<const IStorageProvider>> getProviders() const noexcept override;

	public:
		[[nodiscard]] std::span<const UrlScheme> getUrlScheme() const noexcept override;

		[[nodiscard]] std::span<const u64> getRegisteredUrlScheme() const noexcept override;

	public:
		[[nodiscard]] Arci<IStorage> createStorage(mref<StorageDescriptor> descriptor_) override;

		[[nodiscard]] bool hasStorage(cref<Url> url_) const override;

		[[nodiscard]] Arci<IStorage> getStorageByUrl(cref<Url> url_) const override;

		[[nodiscard]] bool removeStorage(mref<nmpt<IStorage>> storage_) override;
	};
}
