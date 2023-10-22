#include "RenderCommandPool.hpp"

#include <Engine.Common/Make.hpp>

#include "PooledRenderCommandAllocator.hpp"
#include "PooledRenderCommandBuffer.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

ptr<RenderCommandPool::PooledMemory> RenderCommandPool::nextAllocationSegment(
    const pool_buffer_identifier identifier_
) {
    PooledMemoryResource next {
        .acquirer = identifier_,
        .memory = make_uptr<_STD pmr::monotonic_buffer_resource>(resource_base_size, _STD pmr::get_default_resource())
    };

    auto* memory = next.memory.get();
    _segments.push_back(_STD move(next));
    return memory;
}

void RenderCommandPool::resetAllocationSegments(const pool_buffer_identifier identifier_) {

    // Reset Command Lists and Owned Resources
    if (const auto it = _cmdLists.find(identifier_); it != _cmdLists.end()) {
        it->second->clear();
        it->second->shrink_to_fit();
    }

    /*
    if (const auto it = _cmdOwned.find(identifier_); it != _cmdOwned.end()) {
        releaseOwned(*it);
    }
    */

    /**/

    const auto where = _STD ranges::remove_if(
        _segments,
        [identifier_](cref<PooledMemoryResource> segment_) {
            if (segment_.acquirer != identifier_) {
                return false;
            }

            return true;
        }
    );

    // unique_ptr<Ty> will automatically free allocated memory
    _segments.erase(where.begin(), where.end());
}

void RenderCommandPool::erase(const pool_buffer_identifier identifier_) {

    resetAllocationSegments(identifier_);

    _cmdLists.erase(identifier_);
    //_cmdOwned.erase(identifier_);
}

tl::expected<uptr<PooledRenderCommandBuffer>, PoolingError> RenderCommandPool::acquire() noexcept {

    const auto identifier = ++_ids;
    const auto initial = nextAllocationSegment(identifier);

    auto cmdList = make_uptr<Vector<ptr<const RenderCommand>>>();
    auto* const cmdListPtr = cmdList.get();
    _cmdLists.insert(_STD make_pair(identifier, _STD move(cmdList)));

    return make_uptr<PooledRenderCommandBuffer>(
        PooledRenderCommandAllocator {
            this,
            identifier,
            cmdListPtr,
            initial
        }
    );
}

tl::expected<decltype(true), PoolingError> RenderCommandPool::release(
    mref<uptr<PooledRenderCommandBuffer>> resource_
) noexcept {

    auto& allocator = resource_->_allocator;
    const auto identifier = _STD exchange(allocator._identifier, 0);
    [[maybe_unused]] const auto leftMemory = _STD exchange(allocator._poolMemory, nullptr);

    erase(identifier);
    resource_.reset();

    return true;
}

tl::expected<RenderCommandBufferView, PoolingError> RenderCommandPool::view(
    const ptr<const PooledRenderCommandBuffer> buffer_
) const noexcept {

    const auto& allocator = buffer_->_allocator;
    const auto& identifier = allocator._identifier;

    if (const auto it = _cmdLists.find(identifier); it != _cmdLists.end()) {
        return RenderCommandBufferView {
            it->second->begin(),
            it->second->end()
        };
    }

    return tl::make_unexpected(PoolingError {});
}
