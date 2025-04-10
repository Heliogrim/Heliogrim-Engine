#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Resource/File.hpp>

#include "../IStorageFileRepository.hpp"

namespace hg::engine::storage::system {
	class LocalFileStorage;
	class LocalFileSystemProvider;
}

namespace hg::engine::storage::system {
	class EditorReadOnlyRepository final :
		public IStorageFileRepository {
	public:
		friend class ::hg::engine::storage::system::LocalFileSystemProvider;

	public:
		using this_type = EditorReadOnlyRepository;

	protected:
		EditorReadOnlyRepository(
			_In_ ref<LocalFileSystemProvider> provider_,
			mref<::hg::fs::File::path_type> basePath_
		);

	public:
		~EditorReadOnlyRepository() override;

	private:
		nmpt<LocalFileSystemProvider> _lfs;
		::hg::fs::File::path_type _basePath;

	private:
		DenseMap<String, Arci<LocalFileStorage>> _storages;

	public:
		[[nodiscard]] std::span<const nmpt<const IStorageProvider>> getProviders() const noexcept override;

	public:
		[[nodiscard]] std::span<const UrlScheme> getUrlScheme() const noexcept override;

		[[nodiscard]] std::span<const u64> getRegisteredUrlScheme() const noexcept override;

		[[nodiscard]] StringView getVfsMountPoint() const noexcept override;

	public:
		[[nodiscard]] Arci<IStorage> createStorage(mref<StorageDescriptor> descriptor_) override;

		[[nodiscard]] bool hasStorage(cref<Url> url_) const override;

		[[nodiscard]] Arci<IStorage> getStorageByUrl(cref<Url> url_) const override;

		void findReferrerStorages(cref<Arci<IStorage>> ref_, ref<Vector<Arci<IStorage>>> collector_) const override;

		[[nodiscard]] bool removeStorage(mref<nmpt<IStorage>> storage_) override;
	};
}
