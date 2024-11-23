#pragma once
#include "Archive.hpp"

namespace hg::engine::resource {
	class macro_novtable MemoryBaseArchive :
		public Archive {
	public:
		using this_type = MemoryBaseArchive;

	protected:
		MemoryBaseArchive();

		MemoryBaseArchive(this_type&&) noexcept = default;

	public:
		~MemoryBaseArchive() noexcept override = default;

	protected:
		ref<this_type> operator=(this_type&&) noexcept = default;

	protected:
		s64 _pos;

	public:
		void seek(const s64 pos_) final;

		[[nodiscard]] s64 tell() const noexcept final;
	};
}
