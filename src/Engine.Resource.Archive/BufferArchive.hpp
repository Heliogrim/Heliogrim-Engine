#pragma once
#include "MemoryReadWriteArchive.hpp"

namespace hg::engine::resource {
	class BufferArchive :
		public AutoArray<_::byte>,
		public MemoryReadWriteArchive {
	public:
		using this_type = BufferArchive;

	public:
		BufferArchive();
	};
}
