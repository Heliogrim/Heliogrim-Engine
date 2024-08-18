#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>

#include "__fwd.hpp"
#include "Footer/PackageFooter.hpp"
#include "Header/PackageHeader.hpp"

namespace hg::engine::resource {
	class Package final {
	public:
		friend class ::hg::engine::storage::PackageIo;
		friend class ::hg::engine::resource::package::PackageLinker;

	public:
		using this_type = Package;

	public:
		Package(
			_In_ ref<storage::AccessBlobReadWrite> blobAccessor_,
			_In_ mref<PackageHeader> header_,
			_In_ mref<PackageFooter> footer_
		);

		Package(
			_In_ ref<storage::AccessBlobReadonly> blobAccessor_,
			_In_ mref<PackageHeader> header_,
			_In_ mref<PackageFooter> footer_
		);

		Package(mref<this_type>) noexcept = delete;

		Package(cref<this_type>) = delete;

		~Package();

	private:
		Variant<
			nmpt<storage::AccessBlobReadWrite>,
			nmpt<storage::AccessBlobReadonly>
		> _blob;

		PackageHeader _header;
		PackageFooter _footer;

	public:
		[[nodiscard]] cref<PackageHeader> header() const noexcept;

		[[nodiscard]] ref<PackageHeader> header() noexcept;

		[[nodiscard]] cref<PackageFooter> footer() const noexcept;

		[[nodiscard]] ref<PackageFooter> footer() noexcept;
	};
}
