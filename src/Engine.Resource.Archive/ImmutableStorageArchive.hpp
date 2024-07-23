#pragma once
#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	class macro_novtable ImmutableStorageArchive :
		public StorageBaseArchive {
	public:
		using this_type = ImmutableStorageArchive;

	protected:
		ImmutableStorageArchive(
			mref<storage::AccessBlobReadonly> storage_,
			mref<streamoff> offset_,
			mref<streamsize> capacity_
		) :
			StorageBaseArchive(std::move(offset_), std::move(capacity_)),
			_storage(std::move(storage_)) {}

	public:
		~ImmutableStorageArchive() noexcept override = default;

	protected:
		storage::AccessBlobReadonly _storage;
	};
}
