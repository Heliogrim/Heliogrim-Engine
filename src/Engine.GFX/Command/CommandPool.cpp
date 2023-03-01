#include "CommandPool.hpp"
#include "CommandBuffer.hpp"

using namespace hg::engine::gfx;
using namespace hg;

CommandPool::CommandPool(const ptr<CommandQueue>& queue_) noexcept :
    _queue(queue_) { }

void CommandPool::setup() {
    const vk::CommandPoolCreateInfo info {
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        _queue->familyIdx()
    };

    _vkPool = _queue->device()->vkDevice().createCommandPool(info);
    assert(_vkPool);
}

void CommandPool::destroy() {
    if (_vkPool) {
        _queue->device()->vkDevice().destroyCommandPool(_vkPool);
        _vkPool = nullptr;
    }
}

CommandBuffer CommandPool::make() {
    const vk::CommandBufferAllocateInfo info {
        _vkPool,
        vk::CommandBufferLevel::ePrimary,
        1ui32
    };

    auto vkCmds = _queue->device()->vkDevice().allocateCommandBuffers(info);

    assert(!vkCmds.empty());

    return CommandBuffer {
        this,
        vkCmds[0]
    };
}

CommandBuffer CommandPool::make(bool secondary_) {
    const vk::CommandBufferAllocateInfo info {
        _vkPool,
        secondary_ ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary,
        1ui32
    };

    auto vkCmds = _queue->device()->vkDevice().allocateCommandBuffers(info);

    assert(!vkCmds.empty());

    return CommandBuffer {
        this,
        vkCmds[0]
    };
}

void CommandPool::release(const CommandBuffer& buffer_) {
    _queue->device()->vkDevice().freeCommandBuffers(_vkPool, 1, &buffer_.vkCommandBuffer());
}

ptr<CommandQueue> CommandPool::queue() noexcept {
    return _queue;
}

hg::concurrent::UnfairSpinLock& CommandPool::lck() const {
    return _lck;
}
