#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Resource.Blob/Blob.hpp>

#include "PackageLinker.hpp"
#include "ReadonlyLinkedArchive.hpp"
#include "ReadonlyPackageIterator.hpp"

namespace hg::engine::resource {
	class ReadonlyPackageLinker final :
		public PackageLinker {
	public:
		friend class package::ReadonlyPackageIterator;

	public:
		using this_type = ReadonlyPackageLinker;

		using archive_guid_type = Guid;
		using archive_link_type = package::ReadonlyLinkedArchive;
		using iterator_type = package::ReadonlyPackageIterator;

		using link_container_type = DenseMap<archive_guid_type, archive_link_type>;

	public:
		ReadonlyPackageLinker(
			_In_ ref<const Blob> blob_,
			_In_ mref<link_container_type> links_
		) noexcept;

		// TODO: Remove this, add perfect-forward construction as ReadonlyPackage
		ReadonlyPackageLinker(mref<this_type> other_) noexcept = default;

	private:
		ref<const Blob> _blob;
		// Question: Maybe use DenseSet or just Vector<archive_link_type> ?
		link_container_type _links;

	public:
		[[nodiscard]] ref<const Blob> getStorage() const noexcept;

		[[nodiscard]] streamsize computeArchiveSize() const noexcept;

	public:
		[[nodiscard]] bool contains(_In_ cref<archive_guid_type> guid_) const noexcept;

		[[nodiscard]] size_t count() const noexcept;

		[[nodiscard]] Opt<archive_link_type> get(_In_ cref<Guid> guid_) const noexcept;

		[[nodiscard]] Opt<archive_link_type> get(_In_ cref<ArchiveGuid> archiveGuid_) const noexcept;

	public:
		[[nodiscard]] iterator_type begin() const noexcept;

		[[nodiscard]] iterator_type end() const noexcept;

		[[nodiscard]] iterator_type find(_In_ cref<archive_guid_type> guid_) const noexcept;

	public:
		[[nodiscard]] Opt<archive_link_type> operator[](_In_ size_t index_) const noexcept;

		[[nodiscard]] Opt<archive_link_type> operator[](_In_ cref<archive_guid_type> guid_) const noexcept;
	};
}
