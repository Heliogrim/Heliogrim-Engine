#pragma once

#include "MemoryReadWriteArchive.hpp"

namespace hg::engine::resource {
	class BufferArchive :
		public AutoArray<_::byte>,
		public MemoryReadWriteArchive {
	public:
		using this_type = BufferArchive;
		using buffer_type = AutoArray<_::byte>;

	public:
		BufferArchive();

		BufferArchive(const this_type& other_) = delete;

		BufferArchive(this_type&& other_) noexcept;

		~BufferArchive() override = default;

	public:
		this_type& operator=(const this_type&) = delete;

		this_type& operator=(BufferArchive&& other_) noexcept;
	};
}
