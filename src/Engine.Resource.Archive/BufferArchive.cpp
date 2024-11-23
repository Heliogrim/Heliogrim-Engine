#include "BufferArchive.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg::engine::resource;
using namespace hg;

BufferArchive::BufferArchive() :
	AutoArray<_::byte>(),
	MemoryReadWriteArchive(*static_cast<ptr<buffer_type>>(this)) {}

BufferArchive::BufferArchive(this_type&& other_) noexcept :
	AutoArray(::hg::move(other_)),
	MemoryReadWriteArchive(
		static_cast<mref<MemoryBaseArchive>>(other_),
		*static_cast<ptr<buffer_type>>(this),
		other_._limit
	) {}

BufferArchive::this_type& BufferArchive::operator=(BufferArchive&& other_) noexcept {
	if (std::addressof(other_) != this) {
		buffer_type::operator=(::hg::move(other_));
		MemoryReadWriteArchive::operator=(::hg::move(other_));
	}
	return *this;
}
