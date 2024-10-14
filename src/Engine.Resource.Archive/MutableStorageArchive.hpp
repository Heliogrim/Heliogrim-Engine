#pragma once

#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	/**/

	class Blob;

	/**/

	class macro_novtable MutableStorageArchive :
		public StorageBaseArchive {
	public:
		using this_type = MutableStorageArchive;

	protected:
		MutableStorageArchive(
			_Inout_ ref<Blob> storage_,
			_In_ streamoff baseOffset_,
			_In_ streamsize initialSize_
		) :
			StorageBaseArchive(std::move(baseOffset_), std::move(initialSize_)),
			_storage(storage_) {}

	public:
		~MutableStorageArchive() noexcept override = default;

	protected:
		ref<Blob> _storage;

	public:
		[[nodiscard]] s64 totalSize() const noexcept override;
	};
}
