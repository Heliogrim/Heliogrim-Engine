#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Pedantic.Resource/ResourceAccessor.hpp>

#include "ImmutableStorageArchive.hpp"

namespace hg::engine::resource {
	class StorageReadonlyArchive final :
		public ImmutableStorageArchive {
	public:
		using this_type = StorageReadonlyArchive;

	public:
		StorageReadonlyArchive(
			mref<storage::AccessBlobReadonly> storage_,
			mref<streamoff> baseOffset_,
			mref<streamsize> initialSize_
		);

		~StorageReadonlyArchive() override;

	public:
		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
	};
}
