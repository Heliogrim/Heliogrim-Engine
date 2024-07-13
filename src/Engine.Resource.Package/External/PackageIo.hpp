#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Package.hpp"
#include "../Linker/PackageLinker.hpp"

namespace hg::engine::storage {
	class FileUrl;
	class StorageIo;
}

namespace hg::engine::storage {
	class PackageIo final {
	public:
		using this_type = PackageIo;

		using PackageLinker = ::hg::engine::resource::package::PackageLinker;

	public:
		explicit PackageIo(_In_ ref<StorageIo> storageIo_) noexcept;

		constexpr PackageIo(mref<this_type> other_) noexcept = default;

		constexpr PackageIo(cref<this_type> other_) noexcept = default;

		constexpr ~PackageIo() noexcept = default;

	private:
		nmpt<StorageIo> _storageIo;

	public:
		/**
		 *
		 * @blocking
		 *
		 * @param blobAccessor_ The accessor to the underlying data.
		 * @returns a unique pointer to the package above the mutable provided data.
		 */
		// Error: This is error prone, as the accessor will auto-destruct the internal accessor pattern; We may have to take ownership of the managed accessor
		[[nodiscard]] uptr<resource::Package> create_package_from_storage(
			_In_ ref<storage::AccessBlobReadWrite> blobAccessor_
		);

		/**
		 *
		 * @blocking
		 *
		 * @param blobAccessor_ The accessor to the underlying data.
		 * @returns a unique pointer to the package above the mutable provided data.
		 */
		// Error: This is error prone, as the accessor will auto-destruct the internal accessor pattern; We may have to take ownership of the managed accessor
		[[nodiscard]] uptr<resource::Package> create_package_from_storage(
			_In_ ref<storage::AccessBlobReadonly> blobAccessor_
		);

		/**
		 *
		 * @blocking
		 *
		 * @param blobAccessor_ The accessor to the underlying data.
		 * @returns a unique pointer to the package above the immutable provided data.
		 */
		// Error: This is error prone, as the accessor will auto-destruct the internal accessor pattern; We may have to take ownership of the managed accessor
		[[nodiscard]] uptr<resource::Package> create_empty_package(
			_In_ ref<storage::AccessBlobReadWrite> blobAccessor_
		);

		/**
		 *
		 * @blocking
		 * @deprecated
		 *
		 * @param blobAccessor_ The accessor to the underlying data.
		 * @returns a unique pointer to the package above the immutable provided data.
		 */
		// Error: This is error prone, as the accessor will auto-destruct the internal accessor pattern; We may have to take ownership of the managed accessor
		[[nodiscard]] uptr<resource::Package> create_empty_package(
			_In_ ref<storage::AccessBlobReadonly> blobAccessor_
		);

	public:
		ref<resource::Package> readHeader(_In_ ref<resource::Package> package_, _In_ streamoff offset_);

		ref<resource::Package> writeHeader(_In_ ref<resource::Package> package_, _In_ streamoff offset_);

		ref<resource::Package> readFooter(_In_ ref<resource::Package> package_, _In_ streamoff offset_);

		ref<resource::Package> writeFooter(_In_ ref<resource::Package> package_, _In_ streamoff offset_);

	public:
		/**
		 *
		 * @blocking
		 *
		 * @param packageAccessor_ The accessor to the underlying package.
		 * @returns A unique value to the restored linker above the mutable provided package data.
		 */
		[[nodiscard]] UniqueValue<PackageLinker> create_linker_from_package(
			_In_ ref<AccessPackageReadWrite> packageAccessor_
		);

		/**
		 *
		 * @blocking
		 *
		 * @param packageAccessor_ The accessor to the underlying package.
		 * @returns A unique value to the restored linker above the immutable provided package data.
		 */
		[[nodiscard]] UniqueValue<PackageLinker> create_linker_from_package(
			_In_ ref<AccessPackageReadonly> packageAccessor_
		);

		/**
		 *
		 * @blocking
		 *
		 * @param packageAccessor_ The accessor to the assumable empty underlying package.
		 * @returns A unique value to the newly created linker above the mutable provided package data.
		 */
		[[nodiscard]] UniqueValue<PackageLinker> create_empty_linker(
			_In_ ref<AccessPackageReadWrite> packageAccessor_
		);

		/**
		 *
		 * @blocking
		 * @deprecated
		 *
		 * @param packageAccessor_ The accessor to the assumable empty underlying package.
		 * @returns A unique value to the newly created linker above the immutable provided package data.
		 */
		[[nodiscard]] UniqueValue<PackageLinker> create_empty_linker(
			_In_ ref<AccessPackageReadonly> packageAccessor_
		);

		// TODO:
	private:
	public:
		/* Problem: [[deprecated]] */
		ref<PackageLinker> loadLinkerData(_In_ ref<PackageLinker> linker_);

		/* Problem: [[deprecated]] */
		ref<PackageLinker> storeLinkerData(_In_ ref<PackageLinker> linker_);

	public:
		[[deprecated]] void experimental_full_package_load(
			_In_ ref<AccessPackageReadonly> packageAccessor_,
			_Inout_ ref<Vector<uptr<resource::Archive>>> archives_
		);

		[[deprecated]] void experimental_full_package_store(
			_In_ ref<AccessPackageReadWrite> packageAccessor_,
			_In_ mref<Vector<uptr<resource::Archive>>> archives_
		);

		/* Problem: [[deprecated]] */
		[[nodiscard]] bool isPackageFile(
			cref<storage::FileUrl> fileUrl_,
			cref<Arci<IStorage>> fileStorage_
		);

		/* Problem: [[deprecated]] */
		[[nodiscard]] bool isPackageBlob(_In_ ref<AccessBlobReadonly> blob_);
	};
}
