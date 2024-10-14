#pragma once

#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	/**/

	class Blob;

	/**/

	class macro_novtable ImmutableStorageArchive :
		public InheritMeta<ImmutableStorageArchive, StorageBaseArchive> {
	public:
		using this_type = ImmutableStorageArchive;

	protected:
		ImmutableStorageArchive(
			_In_ cref<Blob> storage_,
			_In_ streamoff baseOffset_,
			_In_ streamsize initialSize_
		) :
			InheritMeta(std::move(baseOffset_), std::move(initialSize_)),
			_storage(storage_) {}

	public:
		~ImmutableStorageArchive() noexcept override = default;

	protected:
		cref<Blob> _storage;
	};
}
