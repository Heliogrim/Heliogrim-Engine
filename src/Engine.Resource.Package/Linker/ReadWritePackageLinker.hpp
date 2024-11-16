#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Resource.Blob/Blob.hpp>

#include "PackageLinker.hpp"
#include "ReadWriteLinkedArchive.hpp"
#include "ReadWritePackageIterator.hpp"

namespace hg::engine::resource {
	class ReadWritePackageLinker final :
		public PackageLinker {
	public:
		friend class package::ReadWritePackageIterator;

	public:
		using this_type = ReadWritePackageLinker;

		using archive_link_type = package::ReadWriteLinkedArchive;
		using iterator_type = package::ReadWritePackageIterator;
		using mutable_iterator_type = package::MutableReadWritePackageIterator;

		using link_container_type = StableUnorderedMap<ArchiveGuid, archive_link_type>;

	public:
		explicit ReadWritePackageLinker(
			_Inout_ ref<Blob> blob_,
			_Inout_ mref<link_container_type> links_ = {}
		) noexcept;

		// TODO: Remove this, add perfect-forward construction as ReadWritePackage
		ReadWritePackageLinker(mref<this_type> other_) noexcept = default;

	private:
		ref<Blob> _blob;

		// Attention: Even though Open-Address would result in more compressed memory, we need stable references of the values.
		link_container_type _links;

	public:
		[[nodiscard]] ref<const Blob> getStorage() const noexcept;

		[[nodiscard]] ref<Blob> getStorage() noexcept;

		[[nodiscard]] streamsize computeArchiveSize() const noexcept;

		[[nodiscard]] streamsize computeIndexSize() const noexcept;

	public:
		[[nodiscard]] ref<archive_link_type> add(_Inout_ mref<package::PackageArchiveHeader> header_);

		[[nodiscard]] bool contains(_In_ cref<Guid> guid_) const noexcept;

		[[nodiscard]] size_t count() const noexcept;

		// Question: Should we return const value types or const references?
		[[nodiscard]] Opt<archive_link_type> get(_In_ cref<Guid> guid_) const noexcept;

		[[nodiscard]] Opt<archive_link_type> get(_In_ cref<ArchiveGuid> archiveGuid_) const noexcept;

		[[nodiscard]] Opt<ref<archive_link_type>> get(_In_ cref<ArchiveGuid> archiveGuid_) noexcept;

		void drop(_In_ cref<ArchiveGuid> archiveGuid_) noexcept;

	public:
		[[nodiscard]] iterator_type begin() const noexcept;

		[[nodiscard]] mutable_iterator_type begin() noexcept;

		[[nodiscard]] iterator_type end() const noexcept;

		[[nodiscard]] mutable_iterator_type end() noexcept;

		[[nodiscard]] iterator_type find(_In_ cref<Guid> guid_) const noexcept;

		[[nodiscard]] iterator_type find(_In_ cref<ArchiveGuid> archiveGuid_) const noexcept;

	public:
		// Question: Should we return const value types or const references?
		[[nodiscard]] Opt<archive_link_type> operator[](_In_ size_t index_) const noexcept;

		[[nodiscard]] Opt<archive_link_type> operator[](_In_ cref<Guid> guid_) const noexcept;

		[[nodiscard]] Opt<archive_link_type> operator[](_In_ cref<ArchiveGuid> archiveGuid_) const noexcept;

		[[nodiscard]] Opt<ref<archive_link_type>> operator[](_In_ cref<ArchiveGuid> archiveGuid_) noexcept;

	protected:
	public:
		[[nodiscard]] bool hasChanges() const noexcept;

		[[nodiscard]] Vector<nmpt<package::ReadWriteLinkedArchive>> computeChangeList() noexcept;
	};
}
