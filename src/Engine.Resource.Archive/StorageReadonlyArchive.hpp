#pragma once

#include "ImmutableStorageArchive.hpp"

namespace hg::engine::resource {
	class StorageReadonlyArchive final :
		public InheritMeta<StorageReadonlyArchive, ImmutableStorageArchive> {
	public:
		using this_type = StorageReadonlyArchive;

	public:
		StorageReadonlyArchive(
			_In_ cref<Blob> storage_,
			_In_ streamoff baseOffset_,
			_In_ streamsize initialSize_
		);

		~StorageReadonlyArchive() override;

	public:
		void serializeBytes(ptr<void> value_, u64 size_, ArchiveStreamMode mode_) override;
	};
}
