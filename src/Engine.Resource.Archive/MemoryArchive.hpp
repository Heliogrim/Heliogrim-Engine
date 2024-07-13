#pragma once
#include "Archive.hpp"

namespace hg::engine::resource {
	class __declspec(novtable) MemoryBaseArchive :
		public Archive {
	public:
		using this_type = MemoryBaseArchive;

	protected:
		MemoryBaseArchive();

	protected:
		s64 _pos;

	public:
		void seek(const s64 pos_) final;

		[[nodiscard]] s64 tell() const noexcept final;
	};
}
