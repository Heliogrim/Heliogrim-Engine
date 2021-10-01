#include "CommandQueue.hpp"
#include "CommandBatch.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "../Device/Device.hpp"

using namespace ember::engine::gfx;
using namespace ember;

CommandQueue::CommandQueue(cref<sptr<Device>> device_, const uint32_t idx_, const uint32_t familyIdx_) :
    _device(device_),
    _idx(idx_),
    _familyIdx(familyIdx_),
    _pool(nullptr) {}

void CommandQueue::setup() {
    assert(!_vkQueue);

    _device->vkDevice().getQueue(_familyIdx, _idx, &_vkQueue);
    assert(_vkQueue);

    _pool = new CommandPool(this);
    _pool->setup();
}

void CommandQueue::destroy() {
    if (_pool) {
        _pool->destroy();

        delete _pool;
        _pool = nullptr;
    }

    _vkQueue = nullptr;
}

CommandQueue::operator bool() const {
    return _idx != ~0 && _familyIdx != ~0;
}

void CommandQueue::submit(const CommandBatch& batch_, const vk::Fence& fence_) {

    assert(fence_);

    vector<vk::CommandBuffer> buffers {};
    buffers.resize(batch_.buffers().size());

    for (u32 idx = 0; idx < batch_.buffers().size(); ++idx) {
        buffers[idx] = batch_.buffers()[idx].vkCommandBuffer();
    }

    vk::SubmitInfo info {
        static_cast<u32>(batch_.barriers().size()),
        batch_.barriers().data(),
        &batch_.barrierStages(),
        static_cast<u32>(buffers.size()),
        buffers.data(),
        static_cast<u32>(batch_.signals().size()),
        batch_.signals().data()
    };

    [[maybe_unused]] auto res = _vkQueue.submit(1, &info, fence_);
}

void CommandQueue::submitWait(const CommandBuffer& buffer_) {

    vk::PipelineStageFlags stages { vk::PipelineStageFlagBits::eAllCommands };
    vk::Fence fence = _device->vkDevice().createFence({});

    vk::SubmitInfo info {
        0,
        nullptr,
        &stages,
        1,
        &buffer_.vkCommandBuffer(),
        0,
        nullptr
    };

    [[maybe_unused]] auto submitResult = _vkQueue.submit(1, &info, fence);
    [[maybe_unused]] auto waitResult = _device->vkDevice().waitForFences(1, &fence, true, UINT64_MAX);
    _device->vkDevice().destroyFence(fence);
}

cref<sptr<Device>> CommandQueue::device() const noexcept {
    return _device;
}

u32 CommandQueue::familyIdx() const noexcept {
    return _familyIdx;
}

const vk::Queue& CommandQueue::vkQueue() const noexcept {
    return _vkQueue;
}

ptr<CommandPool> CommandQueue::pool() const noexcept {
    return _pool;
}
