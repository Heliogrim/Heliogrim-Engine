#pragma once

#include <functional>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::storage {
	class Url;
	class IStorage;
	class StorageDescriptor;
}

namespace hg::engine::storage::system {
	class IStorageProvider;
	class IStorageRepository;
}

namespace hg::engine::storage {
	class macro_novtable IStorageRegistry {
	public:
		using this_type = IStorageRegistry;

	protected:
		constexpr IStorageRegistry() = default;

	public:
		IStorageRegistry(cref<this_type>) = delete;

		IStorageRegistry(mref<this_type>) = delete;

		constexpr virtual ~IStorageRegistry() = default;

		/**
		 * Providers
		 */
	public:
		virtual void addProvider(_In_ mref<uptr<system::IStorageProvider>> provider_) = 0;

		virtual bool removeProvider(
			_In_ mref<std::function<bool(nmpt<system::IStorageProvider> provider_)>> selector_
		) = 0;

		/**
		 * Repositories
		 */
	public:
		virtual void addRepository(_In_ nmpt<system::IStorageRepository> repository_) = 0;

		virtual bool removeRepository(
			_In_ mref<std::function<bool(ref<system::IStorageRepository> repository_)>> selector_
		) = 0;

	public:
		[[nodiscard]] virtual bool hasStorage(mref<Url> url_) const noexcept = 0;

		[[nodiscard]] virtual Arci<IStorage> getStorageByUrl(mref<Url> url_) const = 0;

		[[nodiscard]] virtual _Success_(return != nullptr) Arci<IStorage> findStorageByUrl(
			mref<Url> url_
		) const noexcept = 0;

	public:
		[[nodiscard]] virtual _Success_(return != nullptr) Arci<IStorage> insert(
			_In_ mref<StorageDescriptor> descriptor_
		) = 0;

	public:
		[[nodiscard]] virtual _Success_(return != nullptr) Arci<IStorage> removeStorageByUrl(mref<Url> url_) = 0;
	};
}
