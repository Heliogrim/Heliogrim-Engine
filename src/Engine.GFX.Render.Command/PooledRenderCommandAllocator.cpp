#include "PooledRenderCommandAllocator.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

PooledRenderCommandAllocator::PooledRenderCommandAllocator(
    const ptr<RenderCommandPool> pool_,
    const RenderCommandPool::pool_buffer_identifier identifier_,
    const ptr<Vector<ptr<const RenderCommand>>> cmdRecList_,
    const ptr<RenderCommandPool::PooledMemory> initial_
) noexcept :
    RenderCommandAllocator(),
    _identifier(identifier_),
    _pool(pool_),
    _cmdRecList(cmdRecList_),
    _poolMemory(initial_) {}

PooledRenderCommandAllocator::PooledRenderCommandAllocator(mref<this_type> other_) noexcept :
    _identifier(_STD exchange(other_._identifier, ~(decltype(_identifier) { 0 }))),
    _pool(_STD exchange(other_._pool, nullptr)),
    _cmdRecList(_STD exchange(other_._cmdRecList, nullptr)),
    _poolMemory(_STD exchange(other_._poolMemory, nullptr)) {}

PooledRenderCommandAllocator::~PooledRenderCommandAllocator() noexcept {
    assert(_pool == nullptr);
    assert(_poolMemory == nullptr);
}

tl::expected<_STD span<_::byte>, AllocationError> PooledRenderCommandAllocator::allocateUnique(const size_type size_) {

    auto allocated = _poolMemory->allocate(size_);
    if (allocated) {
        _cmdRecList->push_back(static_cast<const ptr<const RenderCommand>>(allocated));
        return _STD span<_::byte> { static_cast<_::byte*>(allocated), size_ };
    }

    const auto nextPoolResource = _pool->nextAllocationSegment(_identifier);
    _poolMemory = nextPoolResource;

    allocated = _poolMemory->allocate(size_);
    if (allocated) {
        _cmdRecList->push_back(static_cast<const ptr<const RenderCommand>>(allocated));
        return _STD span<_::byte> { static_cast<_::byte*>(allocated), size_ };
    }

    return tl::expected<_STD span<_::byte>, AllocationError> { tl::unexpect, AllocationError {} };
}

bool PooledRenderCommandAllocator::deallocateUnique(mref<_STD span<_::byte>> memory_) {
    __debugbreak();
    return false;
}
