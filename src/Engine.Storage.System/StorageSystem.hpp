#pragma once

#include <new>
#include <stdexcept>
#include <system_error>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Resource.Archive/__fwd.hpp>
#include <Engine.Storage.Action/Guard/IoMutationGuard.hpp>
#include <Engine.Storage.Action/Guard/IoQueryGuard.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>
#include <Engine.Storage.Registry/Storage/ArchiveStorage.hpp>
#include <Engine.Storage.Registry/Storage/LocalFileStorage.hpp>
#include <Engine.Storage.Registry/Storage/MemoryStorage.hpp>
#include <Engine.Storage.Registry/Storage/PackageStorage.hpp>
#include <Engine.Storage.Registry/Url/FileUrl.hpp>
#include <Heliogrim/Async/Future.hpp>

#include "Options/ArchiveMutateOptions.hpp"
#include "Options/ArchiveQueryOptions.hpp"
#include "Options/ArchiveRequestOptions.hpp"
#include "Options/BlobMutateOptions.hpp"
#include "Options/BlobQueryOptions.hpp"
#include "Options/FileRequestOptions.hpp"
#include "Options/MemoryRequestOptions.hpp"
#include "Options/PackageMutateOptions.hpp"
#include "Options/PackageQueryOptions.hpp"
#include "Options/PackageRequestOptions.hpp"

namespace hg::engine::resource {
	class ReadonlyPackage;
	class ReadWritePackage;
	class Archive;
	class Blob;
}

namespace hg::engine::storage {
	class IStorageRegistry;
	class StorageActionResolver;

	/**/
	class PackageUrl;
	class MemoryUrl;
	class ArchiveUrl;
}

namespace hg::engine::storage {
	class StorageSystem {
	public:
		using this_type = StorageSystem;

	public:
		explicit StorageSystem(
			_Inout_ ref<IStorageRegistry> registry_,
			_Inout_ ref<StorageActionResolver> resolver_
		) noexcept :
			_registry(registry_),
			_resolver(resolver_),
			_context() {}

		StorageSystem(cref<this_type>) = delete;

		StorageSystem(mref<this_type>) = delete;

		~StorageSystem() noexcept = default;

	protected:
		ref<IStorageRegistry> _registry;
		ref<StorageActionResolver> _resolver;
		Context _context;

	private:
		decltype(auto) indexArchive(auto&& indexer_, auto&& archive_) { return indexer_; }

		decltype(auto) indexPackage(auto&& indexer_, auto&& package_) { return indexer_; }

	public:
		Arci<IStorage> addStorage(_In_ mref<Arci<system::LocalFileStorage>> storage_);

		Arci<IStorage> addStorage(_In_ mref<Arci<system::MemoryStorage>> storage_);

		/**/

		[[nodiscard]] bool hasStorage(mref<ArchiveUrl> archiveUrl_) const noexcept;

		[[nodiscard]] bool hasStorage(mref<FileUrl> fileUrl_) const noexcept;

		[[nodiscard]] bool hasStorage(mref<MemoryUrl> memoryUrl_) const noexcept;

		[[nodiscard]] bool hasStorage(mref<PackageUrl> packageUrl_) const noexcept;

		/**/

		Opt<Arci<IStorage>> removeStorage(mref<ArchiveUrl> archiveUrl_);

		Opt<Arci<IStorage>> removeStorage(mref<FileUrl> fileUrl_);

		Opt<Arci<IStorage>> removeStorage(mref<MemoryUrl> memoryUrl_);

		Opt<Arci<IStorage>> removeStorage(mref<PackageUrl> packageUrl_);

	public:
		Arci<system::PackageStorage> addPackage(_In_ mref<Arci<system::PackageStorage>> storage_);

		[[nodiscard]] Opt<Arci<system::PackageStorage>> findPackage(mref<PackageUrl> packageUrl_) const noexcept;

		[[nodiscard]] bool hasPackage(mref<PackageUrl> packageUrl_) const noexcept;

		Opt<Arci<system::PackageStorage>> removePackage(mref<PackageUrl> packageUrl_);

	public:
		Arci<system::ArchiveStorage> addArchive(_In_ mref<Arci<system::ArchiveStorage>> storage_);

		[[nodiscard]] Opt<Arci<system::ArchiveStorage>> findArchive(mref<ArchiveUrl> archiveUrl_) const noexcept;

		[[nodiscard]] bool hasArchive(mref<ArchiveUrl> archiveUrl_) const noexcept;

		Opt<Arci<system::ArchiveStorage>> removeArchive(mref<ArchiveUrl> archiveUrl_);

	public:
		[[nodiscard]] Result<Arci<system::LocalFileStorage>, request_file_error> requestStorage(
			mref<RequestFileUrlOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::MemoryStorage>, request_memory_error> requestStorage(
			mref<RequestMemoryOptions> options_
		);

		[[nodiscard]] Result<Arci<system::MemoryStorage>, request_memory_located_error> requestStorage(
			mref<MemoryUrl> memoryUrl_,
			mref<RequestMemoryOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::PackageStorage>, request_package_error> requestPackage(
			mref<RequestPackageStorageOptions> options_
		);

		[[nodiscard]] Result<Arci<system::PackageStorage>, request_package_located_error> requestPackage(
			mref<PackageUrl> packageUrl_,
			mref<RequestPackageStorageOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::PackageStorage>, request_package_url_error> requestPackage(
			mref<RequestPackageUrlOptions> options_
		);

		[[nodiscard]] Result<Arci<system::PackageStorage>, request_package_url_located_error> requestPackage(
			mref<PackageUrl> packageUrl_,
			mref<RequestPackageUrlOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_error> requestArchive(
			mref<RequestArchiveStorageOptions> options_
		);

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_located_error> requestArchive(
			mref<ArchiveUrl> archiveUrl_,
			mref<RequestArchiveStorageOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_memory_error> requestArchive(
			mref<RequestArchiveMemoryOptions> options_
		);

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_memory_located_error> requestArchive(
			mref<ArchiveUrl> archiveUrl_,
			mref<RequestArchiveMemoryOptions> options_
		);

		/**/

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_storage_error> requestArchive(
			mref<RequestArchivePackageStorageOptions> options_
		);

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_storage_error> requestArchive(
			mref<RequestArchivePackageUrlOptions> options_
		);

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_storage_located_error> requestArchive(
			mref<ArchiveUrl> archiveUrl_,
			mref<RequestArchivePackageStorageOptions> options_
		);

		[[nodiscard]] Result<Arci<system::ArchiveStorage>, request_archive_storage_located_error> requestArchive(
			mref<ArchiveUrl> archiveUrl_,
			mref<RequestArchivePackageUrlOptions> options_
		);

		/**/

	private:
		[[nodiscard]] query_blob_guard fetchQuery(
			_In_ mref<Arci<IStorage>> storage_
		) const noexcept;

	public:
		auto query(
			_In_ mref<Arci<IStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<const resource::Blob>>, query_blob_error
		> {
			query_blob_guard guard = fetchQuery(::hg::move(storage_));
			return std::invoke(guard, std::forward<decltype(fn_)>(fn_));
		}

		auto queryAsync(
			_In_ mref<Arci<IStorage>> storage_,
			auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), ref<const resource::Blob>>, async_query_blob_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		[[nodiscard]] query_package_guard fetchQuery(
			_In_ mref<Arci<system::PackageStorage>> storage_
		) const noexcept;

	public:
		auto query(
			_In_ mref<Arci<system::PackageStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<const resource::ReadonlyPackage>>, query_package_error
		> {
			query_package_guard guard = fetchQuery(::hg::move(storage_));
			return std::invoke(guard, std::forward<decltype(fn_)>(fn_));
		}

		auto queryAsync(
			_In_ mref<Arci<system::PackageStorage>> storage_,
			auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), ref<const resource::ReadonlyPackage>>, async_query_package_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		[[nodiscard]] query_archive_guard fetchQuery(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		) const noexcept;

	public:
		auto query(
			_In_ mref<Arci<system::ArchiveStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<resource::StorageReadonlyArchive>>, query_archive_error
		> {
			query_archive_guard guard = fetchQuery(::hg::move(storage_));
			return std::invoke(guard, std::forward<decltype(fn_)>(fn_));
		}

		auto queryAsync(
			_In_ mref<Arci<system::ArchiveStorage>> storage_,
			auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), cref<resource::StorageReadonlyArchive>>, async_query_archive_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		[[nodiscard]] mutate_blob_guard fetchMutation(
			_In_ mref<Arci<IStorage>> storage_
		) const noexcept;

	public:
		auto mutate(
			_Inout_ mref<Arci<IStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<resource::Blob>>, mutate_blob_error
		> {
			mutate_blob_guard guard = fetchMutation(::hg::move(storage_));
			return std::invoke(guard, std::forward<decltype(fn_)>(fn_));
		}

		auto mutateAsync(
			[[maybe_unused]] _Inout_ mref<Arci<IStorage>> storage_,
			[[maybe_unused]] auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), ref<resource::Blob>>, async_mutate_blob_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		[[nodiscard]] mutate_package_guard fetchMutation(
			_In_ mref<Arci<system::PackageStorage>> storage_
		) const noexcept;

	public:
		auto mutate(
			_Inout_ mref<Arci<system::PackageStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<resource::ReadWritePackage>>, mutate_package_error
		> {
			mutate_package_guard guard = fetchMutation(::hg::move(storage_));
			return std::invoke(guard, ::hg::forward<decltype(fn_)>(fn_));
		}

		auto mutateAsync(
			_Inout_ mref<Arci<system::PackageStorage>> storage_,
			auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), ref<resource::ReadWritePackage>>, mutate_package_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		[[nodiscard]] mutate_archive_guard fetchMutation(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		) const noexcept;

	public:
		auto mutate(
			_Inout_ mref<Arci<system::ArchiveStorage>> storage_,
			auto&& fn_
		) const noexcept -> Result<
			std::invoke_result_t<decltype(fn_), ref<resource::Archive>>, mutate_archive_error
		> {
			mutate_archive_guard guard = fetchMutation(::hg::move(storage_));
			return std::invoke(guard, std::forward<decltype(fn_)>(fn_));
		}

		auto mutateAsync(
			[[maybe_unused]] _Inout_ mref<Arci<system::ArchiveStorage>> storage_,
			[[maybe_unused]] auto&& fn_
		) const noexcept -> Future<Result<
			std::invoke_result_t<decltype(fn_), ref<resource::Archive>>, async_mutate_archive_error
		>> {
			::hg::todo_panic();
		}

		/**/

	private:
		struct AddArchiveToPackageOptions {
			resource::Archive&& archive;
		};

		Result<Arci<system::ArchiveStorage>, mutate_package_error> addArchiveToPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			mref<AddArchiveToPackageOptions> options_
		);

		/**/

		struct ReplaceArchiveInPackageOptions {
			resource::Archive&& nextArchive;
		};

		Result<Arci<system::ArchiveStorage>, mutate_package_error> replaceArchiveInPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_Inout_ mref<Arci<system::ArchiveStorage>> prevArchive_,
			mref<ReplaceArchiveInPackageOptions> options_
		);

	public:
		template <CompleteType ArchiveType_> requires
			(not std::derived_from<ArchiveType_, resource::StorageArchive>) &&
			std::derived_from<ArchiveType_, resource::Archive> &&
			(not std::is_same_v<ArchiveType_, resource::Archive>)
		decltype(auto) addArchiveToPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_In_ mref<ArchiveType_> archive_
		) {
			return addArchiveToPackage(
				std::move(package_),
				AddArchiveToPackageOptions {
					.archive = static_cast<mref<resource::Archive>>(archive_)
				}
			);
		}

		Result<Arci<system::ArchiveStorage>, Variant<>> addArchiveToPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_In_ mref<Arci<system::ArchiveStorage>> archive_
		);

		/**/

		template <CompleteType ArchiveType_> requires
			(not std::derived_from<ArchiveType_, resource::StorageArchive>) &&
			std::derived_from<ArchiveType_, resource::Archive> &&
			(not std::is_same_v<ArchiveType_, resource::Archive>)
		decltype(auto) replaceArchiveInPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_Inout_ mref<Arci<system::ArchiveStorage>> prevArchive_,
			_In_ mref<ArchiveType_> nextArchive_
		) {
			return replaceArchiveInPackage(
				std::move(package_),
				std::move(prevArchive_),
				ReplaceArchiveInPackageOptions {
					.nextArchive = static_cast<mref<resource::Archive>>(nextArchive_)
				}
			);
		}

		Result<Arci<system::ArchiveStorage>, mutate_package_error> replaceArchiveInPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_Inout_ mref<Arci<system::ArchiveStorage>> prevArchive_,
			_In_ mref<Arci<system::ArchiveStorage>> nextArchive_
		);

		/**/

		Result<Arci<system::PackageStorage>, mutate_package_error> removeArchiveFromPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_Inout_ mref<Arci<system::ArchiveStorage>> archive_
		);

		Result<Arci<system::PackageStorage>, mutate_package_error> removeArchiveFromPackage(
			_Inout_ mref<Arci<system::PackageStorage>> package_,
			_Inout_ mref<ArchiveUrl> archiveUrl_
		);

		/**/

		Result<Vector<Arci<system::ArchiveStorage>>, query_blob_error> enumArchivesFromPackage(
			_In_ mref<Arci<system::PackageStorage>> package_
		);
	};
}

/**/

#include "Details/ExtendedStorageSystem.hpp"
