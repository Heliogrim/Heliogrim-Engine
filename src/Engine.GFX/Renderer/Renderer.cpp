#include "Renderer.hpp"

#include <Engine.Scheduler/Thread/Thread.hpp>

#include "RenderInvocation.hpp"
#include "RenderPipeline.hpp"

using namespace ember::engine::gfx;
using namespace ember;

void Renderer::setup(cref<sptr<Device>> device_) {
    _device = device_;
}

cref<sptr<Device>> Renderer::device() const noexcept {
    return _device;
}

sptr<RenderInvocationState> Renderer::makeInvocationState() const {
    // TODO:
    return make_sptr<RenderInvocationState>();
}

ptr<RenderInvocation> Renderer::allocate(mref<RenderInvocationData> invocation_) {
    auto* invocation {
        new RenderInvocation {
            this,
            _STD move(invocation_),
            nullptr
        }
    };

    auto state { makeInvocationState() };

    pipeline()->allocateWith(invocation, state.get());

    const auto& targetStage { invocation->state() };
    const_cast<ref<sptr<RenderInvocationState>>>(targetStage).swap(state);

    return invocation;
}

void Renderer::free(mref<ptr<RenderInvocation>> invocation_) {

    /**
     *
     */
    pipeline()->freeWith(invocation_, invocation_->state().get());

    /**
     * Warning: Temporary Cleanup for Enforced Synchronization
     */
    for (const auto& entry : invocation_->lastSignals()) {
        _device->vkDevice().destroySemaphore(entry);
    }

    /**
     *
     */
    delete invocation_;
    invocation_ = nullptr;
}

void Renderer::invokeBatched(const non_owning_rptr<RenderInvocation> invocation_, mref<CommandBatch> batch_) const {

    #ifdef _DEBUG
    assert(invocation_->renderer() == this);
    assert(invocation_->state() && invocation_->state()->framebuffer);
    #endif

    /**
     *
     */
    pipeline()->process(invocation_, batch_);

    /**
     * Warning: Temporary Enforced Submission Order
     */
    #ifdef _DEBUG
    assert(!batch_.buffers().empty());
    #endif

    const auto requiredSignals { batch_.buffers().size() - 1ui32 };
    if (requiredSignals > invocation_->lastSignals().size()) {
        for (u64 i = invocation_->lastSignals().size(); i < requiredSignals; ++i) {
            invocation_->lastSignals().push_back(
                _device->vkDevice().createSemaphore({
                    vk::SemaphoreCreateFlags {}
                })
            );
        }
    }

    /**
     *
     */
    vk::Fence fence { invocation_->unsafeSync() };
    if (!fence) {
        fence = { _device->vkDevice().createFence(vk::FenceCreateInfo {}) };
        #ifdef _DEBUG
        assert(invocation_->storeSync(vk::Fence { fence }));
        #else
        invocation_->storeSync(_STD move(fence));
        #endif

    } else {
        _device->vkDevice().resetFences(1, &fence);
    }

    // Warning: Temporary Commented, cause enforced sync need special treatment
    #if FALSE
    /**
     *
     */
    auto* queue { _device->graphicsQueue() };
    queue->submit(batch_, fence);
    #endif

    auto* queue { _device->graphicsQueue() };

    // First Enforced
    vk::SubmitInfo info {
        static_cast<u32>(batch_.barriers().size()),
        batch_.barriers().data(),
        &batch_.barrierStages(),
        1ui32,
        &batch_.buffers()[0].vkCommandBuffer(),
        0ui32,
        nullptr
    };

    if (&batch_.buffers().front() == &batch_.buffers().back()) {
        // Front == Back ~ only one submission
        info.signalSemaphoreCount = static_cast<u32>(batch_.signals().size());
        info.pSignalSemaphores = batch_.signals().data();

        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, fence) };

    } else {
        info.signalSemaphoreCount = 1ui32;
        info.pSignalSemaphores = &invocation_->lastSignals()[0];
        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, vk::Fence {}) };
    }

    // Dynamic Enforced
    for (u32 i = 1; i < requiredSignals; ++i) {

        info.waitSemaphoreCount = 1ui32;
        info.pWaitSemaphores = &invocation_->lastSignals()[i - 1ui32];
        vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        info.pWaitDstStageMask = &waitStage;

        info.pCommandBuffers = &batch_.buffers()[i].vkCommandBuffer();

        info.signalSemaphoreCount = 1ui32;
        info.pSignalSemaphores = &invocation_->lastSignals()[i];

        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, vk::Fence {}) };
    }

    // Last Enforced
    if (&batch_.buffers().front() != &batch_.buffers().back()) {
        // Front != Back ~ at least two submissions
        info.waitSemaphoreCount = 1ui32;
        info.pWaitSemaphores = &invocation_->lastSignals()[requiredSignals - 1ui32];
        vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        info.pWaitDstStageMask = &waitStage;

        info.pCommandBuffers = &batch_.buffers().back().vkCommandBuffer();

        info.signalSemaphoreCount = static_cast<u32>(batch_.signals().size());
        info.pSignalSemaphores = batch_.signals().data();

        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, fence) };
    }
}

const non_owning_rptr<RenderInvocation> Renderer::invoke(const non_owning_rptr<RenderInvocation> invocation_) {
    /**
     * Forward Invocation with default CommandBatch
     */
    this->invokeBatched(invocation_, {});
    return invocation_;
}

const non_owning_rptr<RenderInvocation> Renderer::invoke(const non_owning_rptr<RenderInvocation> invocation_,
    cref<CommandBatch> batchLayout_) {

    #ifdef _DEBUG
    // Ensure batch layout does not contain any command buffers
    assert(batchLayout_.buffers().empty());
    #endif

    /**
     * Forward Invocation with custom CommandBatch
     */
    this->invokeBatched(invocation_, CommandBatch { batchLayout_ });
    return invocation_;
}
