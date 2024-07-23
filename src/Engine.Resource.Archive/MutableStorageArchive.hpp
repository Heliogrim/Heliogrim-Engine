#pragma once
#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	class __declspec(novtable) MutableStorageArchive :
		public StorageBaseArchive {
	public:
		using this_type = MutableStorageArchive;

	protected:
		MutableStorageArchive(
			mref<storage::AccessBlobReadWrite> storage_,
			mref<streamoff> offset_,
			mref<streamsize> capacity_
		) :
			StorageBaseArchive(std::move(offset_), std::move(capacity_)),
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
