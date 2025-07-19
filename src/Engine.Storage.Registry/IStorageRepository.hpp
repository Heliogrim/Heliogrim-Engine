#pragma once

#include <span>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::storage {
	class StorageUrl;
	class UrlScheme;
	class IStorage;
	class StorageDescriptor;
}

namespace hg::engine::storage::system {
	class IStorageProvider;
}

namespace hg::engine::storage::system {
	class macro_novtable IStorageRepository {
	public:
		using this_type = IStorageRepository;

	protected:
		constexpr IStorageRepository() noexcept = default;

	public:
		IStorageRepository(cref<this_type>) = delete;

		IStorageRepository(mref<this_type>) = delete;

		constexpr virtual ~IStorageRepository() = default;

	public:
		[[nodiscard]] virtual std::span<const nmpt<const IStorageProvider>> getProviders() const noexcept = 0;

	public:
		/**
		 * 
		 * @returns A span of string views encoding the assoctiated url schemes of this repository.
		 */
		[[nodiscard]] virtual std::span<const UrlScheme> getUrlScheme() const noexcept = 0;

		/**
		 * 
		 * @returns A span of u64 values encoding globally uniquely registered url schemes.
		 */
		[[nodiscard]] virtual std::span<const u64> getRegisteredUrlScheme() const noexcept = 0;

	public:
		[[nodiscard]] _Success_(return != nullptr) virtual Arci<IStorage> createStorage(
			mref<StorageDescriptor> descriptor_
		) = 0;

		[[nodiscard]] virtual bool hasStorage(cref<StorageUrl> url_) const = 0;

		[[nodiscard]] virtual Arci<IStorage> getStorageByUrl(cref<StorageUrl> url_) const = 0;

		virtual void findReferrerStorages(_In_ cref<Arci<IStorage>> ref_, _Inout_ ref<Vector<Arci<IStorage>>> collector_) const = 0;

		[[nodiscard]] virtual bool removeStorage(_In_ mref<nmpt<IStorage>> storage_) = 0;
	};
}
