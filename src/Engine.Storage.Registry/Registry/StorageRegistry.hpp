#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Utils/_CTRL.hpp>

#include "../IStorageRegistry.hpp"

namespace hg::engine {
	class Config;
}

namespace hg::engine::storage {
	class StorageRegistry final :
		public IStorageRegistry {
	public:
		using this_type = StorageRegistry;

	public:
		StorageRegistry() noexcept;

		~StorageRegistry() noexcept override;

	public:
		void setup(cref<Config> config_);

		void tidy();

	private:
		mutable _SCTRL_OBJ(_mtx);
		Vector<uptr<system::IStorageProvider>> _provider;
		Vector<nmpt<system::IStorageRepository>> _repositories;

	public:
		void addProvider(mref<uptr<system::IStorageProvider>> provider_) override;

		bool removeProvider(mref<std::function<bool(nmpt<system::IStorageProvider> provider_)>> selector_) override;

	public:
		void addRepository(_In_ nmpt<system::IStorageRepository> repository_) override;

		bool removeRepository(
			mref<std::function<bool(ref<system::IStorageRepository> repository_)>> selector_
		) override;

	public:
		[[nodiscard]] Arci<IStorage> insert(mref<StorageDescriptor> descriptor_) override;

	public:
		[[nodiscard]] bool hasStorage(mref<StorageUrl> url_) const noexcept override;

		[[nodiscard]] Arci<IStorage> getStorageByUrl(mref<StorageUrl> url_) const override;

		[[nodiscard]] Arci<IStorage> findStorageByUrl(mref<StorageUrl> url_) const noexcept override;

		bool findReferrerStorages(
			mref<UrlScheme> scheme_,
			cref<Arci<IStorage>> ref_,
			ref<Vector<Arci<IStorage>>> collector_
		) const override;

	public:
		[[nodiscard]] Arci<IStorage> removeStorageByUrl(mref<StorageUrl> url_) override;
	};
}
