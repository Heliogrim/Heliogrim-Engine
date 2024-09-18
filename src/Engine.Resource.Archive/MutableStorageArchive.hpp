#pragma once
#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	class macro_novtable MutableStorageArchive :
		public StorageBaseArchive {
	public:
		using this_type = MutableStorageArchive;

	protected:
		MutableStorageArchive(
			mref<storage::AccessBlobReadWrite> storage_,
			mref<streamoff> baseOffset_,
			mref<streamsize> initialSize_
		) :
			StorageBaseArchive(std::move(baseOffset_), std::move(initialSize_)),
			_storage(std::move(storage_)) {}

	public:
		~MutableStorageArchive() noexcept override = default;

	protected:
		storage::AccessBlobReadWrite _storage;

	public:
		[[nodiscard]] s64 totalSize() const noexcept override;

		[[nodiscard]] storage::AccessBlobReadWrite release() && {
			// BUG: Breaks
			return std::move(_storage);
		}
	};
}
