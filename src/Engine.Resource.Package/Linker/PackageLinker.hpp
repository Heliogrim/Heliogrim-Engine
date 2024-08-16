#pragma once

#include <functional>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Pedantic.Resource/Resource.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>

#include "LinkedArchive.hpp"
#include "__fwd.hpp"
#include "../__fwd.hpp"

namespace hg::engine::resource {
	class StorageBaseArchive;
	class StorageReadonlyArchive;
	class StorageReadWriteArchive;
}

namespace hg::engine::resource::package {
	class PackageLinker final {
	public:
		friend class storage::PackageIo;

		template <bool Const_>
		friend class LinkedArchiveIterator;

	public:
		using this_type = PackageLinker;

		using iterator_type = LinkedArchiveIterator<false>;
		using const_iterator_type = LinkedArchiveIterator<true>;

		using iter_archive = StorageBaseArchive;
		using readonly_iter_archive = StorageReadonlyArchive;

		using tracked_locked_writer = uptr<StorageReadWriteArchive, std::function<void(ptr<StorageReadWriteArchive>)>>;

	public:
		PackageLinker(
			_In_ cref<storage::StorageIo> storageIo_,
			_In_ ref<storage::AccessPackageReadWrite> packageAccessor_
		);

		PackageLinker(
			_In_ cref<storage::StorageIo> storageIo_,
			_In_ ref<storage::AccessPackageReadonly> packageAccessor_
		);

		PackageLinker(cref<this_type>) = delete;

		constexpr PackageLinker(mref<this_type> other_) noexcept = default;

		~PackageLinker();

	private:
		nmpt<const storage::StorageIo> _storageIo;
		Variant<
			nmpt<storage::AccessPackageReadWrite>,
			nmpt<storage::AccessPackageReadonly>
		> _package;

	public:
		[[nodiscard]] bool canLoad() const noexcept;

		[[nodiscard]] bool canStore() const noexcept;

	private:
		Vector<LinkedArchive> _links;

	public:
		bool store(mref<PackageArchiveHeader> header_, mref<uptr<Archive>> archive_);

		[[nodiscard]] _Success_(return != nullptr) tracked_locked_writer store(
			mref<PackageArchiveHeader> header_,
			std::try_to_lock_t = std::try_to_lock
		);

		template <typename Fn_> requires std::is_invocable_v<Fn_, mref<tracked_locked_writer>>
		decltype(auto) store(_In_ mref<PackageArchiveHeader> header_, Fn_&& genFn_) {
			// TODO: Check whether we can optimize and maybe queue actions
			return std::forward<Fn_>(genFn_)(store(std::move(header_), std::try_to_lock));
		}

		/**
		 * Try to get a valid readonly storage archive instance.
		 *
		 * @details This function will only query against the internally stored state of the linker. Therefore it is
		 *	necessary to dispatch a linker data load operation beforehand, otherwise this function will fail to query
		 *	the correct archive data.
		 *
		 * @note There is no valid load operation to generate mutable storage archive references as they are prone to
		 *	generate an invalid state of the underlying package and linker object.
		 *
		 * @param archiveGuid_ The key used to query for a valid readable archive within the underlying package or linker state.
		 * @returns A pointer to a unique readonly storage archive instance.
		 */
		[[nodiscard]] _Success_(return != nullptr) uptr<StorageReadonlyArchive> load(
			_In_ mref<Guid> archiveGuid_
		) const noexcept;

		[[deprecated]] void remove(mref<Guid> archiveGuid_);

	public:
		[[nodiscard]] bool containsArchive(mref<Guid> archiveGuid_) const noexcept;

		[[nodiscard]] u64 getArchiveCount() const noexcept;

		[[nodiscard]] s64 getArchiveSize() const noexcept;

	public:
		[[nodiscard]] const_iterator_type begin() const noexcept;

		[[nodiscard]] iterator_type begin() noexcept;

		[[nodiscard]] const_iterator_type cbegin() const noexcept;

		[[nodiscard]] const_iterator_type end() const noexcept;

		[[nodiscard]] iterator_type end() noexcept;

		[[nodiscard]] const_iterator_type cend() const noexcept;

	protected:
		[[nodiscard]] uptr<readonly_iter_archive> operator[](const_iterator_type where_) const noexcept;

		// Warning: Do not expose this as long as we don't have a good solution of tracking internal memory changes.
		[[nodiscard]] uptr<iter_archive> operator[](iterator_type where_) const noexcept;

		[[nodiscard]] uptr<readonly_iter_archive> operator[](cref<LinkedArchive> linked_) const noexcept;
	};
}
