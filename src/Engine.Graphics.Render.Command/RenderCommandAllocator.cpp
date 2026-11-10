#include "RenderCommandAllocator.hpp"

#include <memory_resource>
#include <Engine.Asserts/Todo.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg::engine::render::cmd;
using namespace hg;

RenderCommandAllocator::RenderCommandAllocator() noexcept = default;

RenderCommandAllocator::RenderCommandAllocator(mref<this_type> other_) noexcept :
	_memory(std::move(other_._memory)) {}

RenderCommandAllocator::~RenderCommandAllocator() noexcept {
	assert(_memory.empty());
}

void RenderCommandAllocator::release() {

	for (auto&& memory : _memory) {
		memory.reset();
	}

	_memory.clear();
}

void* RenderCommandAllocator::allocateUnique(const size_type size_) {

	void* allocated = nullptr;
	if (_memory.empty() || (allocated = _memory.back()->allocate(size_)) == nullptr) {

		_memory.emplace_back(
			make_uptr<std::pmr::monotonic_buffer_resource>(initial_memory_size, std::pmr::get_default_resource())
		);
		allocated = _memory.back()->allocate(size_);

	}

	return allocated;
}

bool RenderCommandAllocator::deallocateUnique(mref<void*> memory_) {
	::hg::todo_panic();
	return false;
}
