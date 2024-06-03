#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "../IStorageRepository.hpp"

namespace hg::engine::storage::system {
	class MemoryStorage;
}

namespace hg::engine::storage::system {
	class MemoryRepository final :
		public IStorageRepository {
	public:
		friend class MemoryProvider;

	public:
		using this_type = MemoryRepository;

	protected:
		explicit MemoryRepository(_In_ ref<MemoryProvider> provider_);

	public:
		~MemoryRepository() override;

	private:
		nmpt<MemoryProvider> _provider;
		DenseMap<Guid, Arci<MemoryStorage>> _storages;

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
