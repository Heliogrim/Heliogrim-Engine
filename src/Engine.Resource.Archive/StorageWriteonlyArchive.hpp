#pragma once

#include "MutableStorageArchive.hpp"
#include "StorageBaseArchive.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class StorageWriteonlyArchive final :
		public MutableStorageArchive {
	public:
		using this_type = StorageWriteonlyArchive;

	public:
		StorageWriteonlyArchive(
			_Inout_ ref<Blob> storage_,
			_In_ streamoff baseOffset_,
			_In_ streamsize initialSize_
		);

		~StorageWriteonlyArchive() override;

	public:
		void serializeBytes(ptr<void> value_, u64 size_, ArchiveStreamMode mode_) override;
	};
}
