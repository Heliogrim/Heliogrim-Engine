#pragma once
#include "MutableStorageArchive.hpp"
#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	class StorageReadWriteArchive final :
		public MutableStorageArchive {
	public:
		using this_type = StorageReadWriteArchive;

	public:
		StorageReadWriteArchive(
			mref<storage::AccessBlobReadWrite> storage_,
			mref<streamoff> offset_,
			mref<streamsize> capacity_
		);

		~StorageReadWriteArchive() override;

	public:
		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
	};
}
