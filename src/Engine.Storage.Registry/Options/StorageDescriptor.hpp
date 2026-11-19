#pragma once

#include <Engine.Common/__macro.hpp>

#include "ArchiveStorageDescriptor.hpp"
#include "FileStorageDescriptor.hpp"
#include "MemoryStorageDescriptor.hpp"
#include "PackageStorageDescriptor.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
import Engine.Common;
#endif

namespace hg::engine::storage {
	template <class Type_>
	concept IsStorageDescriptor = IsAnyOf<
		Type_,
		ArchiveStorageDescriptor,
		FileStorageDescriptor,
		MemoryStorageDescriptor,
		PackageStorageDescriptor
	>;

	/**/

	class StorageDescriptor :
		public Variant<
			ArchiveStorageDescriptor,
			FileStorageDescriptor,
			MemoryStorageDescriptor,
			PackageStorageDescriptor
		> {
	public:
		using this_type = StorageDescriptor;
		using underlying_type = Variant<
			ArchiveStorageDescriptor,
			FileStorageDescriptor,
			MemoryStorageDescriptor,
			PackageStorageDescriptor
		>;

	public:
		using underlying_type::Variant;

	public:
		[[nodiscard]] constexpr UrlScheme targetScheme() const {
			::hg::assertrt(not this->valueless_by_exception());
			return invoke_from_shared_type([](const auto& val_) { return val_.targetScheme(); });
		}

		[[nodiscard]] constexpr bool valid() const noexcept {
			return (not this->valueless_by_exception()) &&
				invoke_from_shared_type([](const auto& val_) { return val_.valid(); });
		}
	};
}
