#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "Archive.hpp"

namespace hg::engine::resource {
	class macro_novtable StorageBaseArchive :
		public InheritBase<StorageBaseArchive>,
		public Archive {
	public:
		using this_type = StorageBaseArchive;

	protected:
		StorageBaseArchive(
			mref<streamoff> baseOffset_,
			mref<streamsize> initialSize_
		);

	public:
		StorageBaseArchive(cref<this_type>) = delete;

		StorageBaseArchive(mref<this_type>) = delete;

		~StorageBaseArchive() noexcept override;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;

	protected:
		streamoff _baseOffset;
		streamsize _size;
		s64 _cursor;

	public:
		void seek(s64 pos_) override;

		[[nodiscard]] s64 tell() const noexcept override;
	};
}
