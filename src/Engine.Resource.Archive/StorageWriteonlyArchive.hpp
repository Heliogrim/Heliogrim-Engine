#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Pedantic.Resource/ResourceAccessor.hpp>

#include "MutableStorageArchive.hpp"
#include "StorageBaseArchive.hpp"

namespace hg::engine::resource {
	class StorageWriteonlyArchive final :
		public MutableStorageArchive {
	public:
		using this_type = StorageWriteonlyArchive;

	public:
		StorageWriteonlyArchive(
			mref<storage::AccessBlobReadWrite> storage_,
			mref<streamoff> offset_,
			mref<streamsize> capacity_
		);

		~StorageWriteonlyArchive() override;

	public:
		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
	};
}
