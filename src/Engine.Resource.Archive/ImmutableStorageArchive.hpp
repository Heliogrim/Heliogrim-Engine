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
			mref<streamoff> baseOffset_,
			mref<streamsize> initialSize_
		) :
			StorageBaseArchive(std::move(baseOffset_), std::move(initialSize_)),
			_storage(std::move(storage_)) {}

	public:
		~ImmutableStorageArchive() noexcept override = default;

	protected:
		storage::AccessBlobReadonly _storage;
	};
}
