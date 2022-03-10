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
     *
     */
    delete invocation_;
    invocation_ = nullptr;
}

void Renderer::invokeBatched(const non_owning_rptr<RenderInvocation> invocation_, mref<CommandBatch> batch_) {

    #ifdef _DEBUG
    assert(invocation_->renderer() == this);
    assert(invocation_->state() && invocation_->state()->framebuffer);
    #endif

    /**
     *
     */
    pipeline()->process(invocation_, batch_);

    /**
     *
     */
    vk::Fence fence { invocation_->unsafeSync() };
    if (!fence) {
        fence = { _device->vkDevice().createFence(vk::FenceCreateInfo {}) };
        #ifdef _DEBUG
        assert(invocation_->storeSync(_STD move(fence)));
        #else
        invocation_->storeSync(_STD move(fence));
        #endif

    } else {
        _device->vkDevice().resetFences(1, &fence);
    }

    /**
     *
     */
    auto* queue { _device->graphicsQueue() };
    queue->submit(batch_, fence);
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
