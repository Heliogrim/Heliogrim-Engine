#include "CommandBatch.hpp"

using namespace hg::engine::gfx;
using namespace hg;

CommandBatch::CommandBatch(const CommandBuffer& buffer_) :
    _buffers({ buffer_ }),
    _barriers(),
    _barrierStages(),
    _signals() {}

CommandBatch::CommandBatch(mref<Vector<CommandBuffer>> buffers_) :
    _buffers(_STD move(buffers_)),
    _barriers(),
    _barrierStages(),
    _signals() {}

CommandBatch::CommandBatch(cref<this_type> other_) :
    _buffers(other_._buffers),
    _barriers(other_._barriers),
    _barrierStages(other_._barrierStages),
    _signals(other_._signals) {}

bool CommandBatch::empty() const noexcept {
    return _buffers.empty() && _signals.empty();
}

void CommandBatch::reset() {
    _barriers.clear();
    _barrierStages.clear();
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

const Vector<vk::Semaphore>& CommandBatch::barriers() const noexcept {
    return _barriers;
}

cref<Vector<vk::PipelineStageFlags>> CommandBatch::barrierStages() const noexcept {
    return _barrierStages;
}

void CommandBatch::pushBarrier(const vk::Semaphore& barrier_, cref<vk::PipelineStageFlags> stage_) {
    _barriers.push_back(barrier_);
    _barrierStages.push_back(stage_);
}

const Vector<vk::Semaphore>& CommandBatch::signals() const noexcept {
    return _signals;
}

void CommandBatch::pushSignal(const vk::Semaphore& signal_) {
    _signals.push_back(signal_);
}
