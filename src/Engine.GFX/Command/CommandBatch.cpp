#include "CommandBatch.hpp"

using namespace ember::engine::gfx;

void CommandBatch::reset() {
    _barriers.clear();
    _barrierStages = vk::PipelineStageFlags {};
    _buffers.clear();
    _signals.clear();
}

const std::vector<CommandBuffer>& CommandBatch::buffers() const noexcept {
    return _buffers;
}

void CommandBatch::push(CommandBuffer&& buffer_) {
    _buffers.push_back(_STD forward<CommandBuffer>(buffer_));
}

void CommandBatch::push(const CommandBuffer& buffer_) {
    _buffers.push_back(buffer_);
}

const std::vector<vk::Semaphore>& CommandBatch::barriers() const noexcept {
    return _barriers;
}

void CommandBatch::pushBarrier(const vk::Semaphore& barrier_) {
    _barriers.push_back(barrier_);
}

const vk::PipelineStageFlags& CommandBatch::barrierStages() const noexcept {
    return _barrierStages;
}

vk::PipelineStageFlags& CommandBatch::barrierStages() noexcept {
    return _barrierStages;
}

const std::vector<vk::Semaphore>& CommandBatch::signals() const noexcept {
    return _signals;
}

void CommandBatch::pushSignal(const vk::Semaphore& signal_) {
    _signals.push_back(signal_);
}
