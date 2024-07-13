#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Pedantic.Resource/Resource.hpp>
#include <Engine.Pedantic.Resource/ResourceAccessor.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Resource.Package/Package.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>

#include "Archive.hpp"

namespace hg::engine::resource {
	class __declspec(novtable) StorageBaseArchive :
		public Archive {
	public:
		using this_type = StorageBaseArchive;

	protected:
		StorageBaseArchive(
			mref<streamoff> offset_,
			mref<streamsize> capacity_
		);

	public:
		StorageBaseArchive(cref<this_type>) = delete;

		StorageBaseArchive(mref<this_type>) = delete;

		~StorageBaseArchive() noexcept override;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;

	protected:
		streamoff _offset;
		streamsize _capacity;
		s64 _cursor;

	public:
		void seek(const s64 pos_) override;

		[[nodiscard]] s64 tell() const noexcept override;
	};
}
