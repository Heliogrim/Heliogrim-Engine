#pragma once
#include "MemoryReadWriteArchive.hpp"

namespace hg::engine::resource {
	class BufferArchive :
		public MemoryReadWriteArchive,
		public AutoArray<_::byte> {
	public:
		using this_type = BufferArchive;

	public:
		BufferArchive();
	};
}
