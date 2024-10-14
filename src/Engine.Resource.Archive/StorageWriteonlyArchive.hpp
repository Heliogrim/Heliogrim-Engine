#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "MutableStorageArchive.hpp"
#include "StorageBaseArchive.hpp"

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
