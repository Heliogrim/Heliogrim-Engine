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
			_Inout_ ref<Blob> storage_,
			_In_ streamoff baseOffset_,
			_In_ streamsize initialSize_
		);

		~StorageReadWriteArchive() override;

	public:
		void serializeBytes(ptr<void> value_, u64 size_, ArchiveStreamMode mode_) override;
	};
}
