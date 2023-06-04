#include "Renderer.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Command/CommandBatch.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "HORenderPass.hpp"
#include "RenderPassState.hpp"
#include "RenderPipeline.hpp"

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

Renderer::Renderer() noexcept :
    _device(nullptr),
    _pipeline(nullptr) {}

Renderer::~Renderer() {
    /**
     *
     */
    if (_pipeline) {
        delete _pipeline;
        _pipeline = nullptr;
    }
}

void Renderer::setup(cref<sptr<Device>> device_) {
    _device = device_;

    #ifdef _DEBUG
    const auto validationResult { _pipeline->validate() };
    // Warning: TODO: Validate pipeline
    //assert(validationResult == RenderPipelineValidationResult::eSuccess);
    #else
    const auto validationResult { _pipeline->validate() };
    if (validationResult != RenderPipelineValidationResult::eSuccess) {
        // TODO: Logger :: warn(...)
    }
    #endif

    _pipeline->setup(device_);
}

cref<sptr<Device>> Renderer::device() const noexcept {
    return _device;
}

sptr<RenderPassState> Renderer::makeRenderPassState() const {

    auto* const globalCacheCtrl { Engine::getEngine()->getGraphics()->cacheCtrl() };

    auto localCache { make_uptr<cache::LocalResourceCache>(globalCacheCtrl) };

    auto state = new RenderPassState {
        .device = _device,
        .framebuffer = nullptr,
        .cacheCtrl = decltype(RenderPassState::cacheCtrl) { _STD move(localCache) },
        .alloc = decltype(RenderPassState::alloc) { _device->allocator() },
        .bindingCache = DiscreteBindingCache { _device },
        .data = decltype(RenderPassState::data) {}
    };

    // TODO:
    return sptr<RenderPassState>(state);
}

ptr<HORenderPass> Renderer::allocate(mref<HORenderPassCreateData> data_) {
    auto* renderPass {
        new HORenderPass {
            this,
            _STD move(data_),
            makeRenderPassState()
        }
    };

    _pipeline->allocate(renderPass);
    return renderPass;
}

ptr<HORenderPass> Renderer::reallocate(mref<ptr<HORenderPass>> renderPass_, mref<HORenderPassChanges> changes_) {

    if (renderPass_->renderer() != this) {
        return nullptr;
    }

    // Warning: Temporary implementation
    HORenderPassCreateData tmp { changes_.target, renderPass_->scene(), renderPass_->sceneView() };
    free(_STD move(renderPass_));

    return allocate(_STD move(tmp));
}

bool Renderer::free(mref<ptr<HORenderPass>> renderPass_) {

    if (renderPass_->renderer() != this) {
        return false;
    }

    /**
     *
     */
    _pipeline->free(renderPass_);

    #ifdef _DEBUG
    if (!renderPass_->state()->data.empty() || renderPass_->state()->framebuffer != nullptr) {
        IM_CORE_WARN("Failed to free all HORenderPass resources.");
        __debugbreak();
    }
    #endif

    /**
     * Warning: Temporary Cleanup for Enforced Synchronization
     */
    for (const auto& entry : renderPass_->lastSignals()) {
        _device->vkDevice().destroySemaphore(entry);
    }

    /**
     *
     */
    delete renderPass_;
    renderPass_ = nullptr;

    return true;
}

void Renderer::invokeBatched(const non_owning_rptr<HORenderPass> renderPass_, mref<CommandBatch> batch_) const {

    #if defined(_DEBUG) || TRUE
    assert(renderPass_->renderer() == this);
    // assert(renderPass_->state() && renderPass_->state()->framebuffer);
    renderPass_->batches().clear();
    #endif

    /**
     * Warning: Temporary => Validate usability of provided state
     */
    if (!renderPass_->isReset()) {
        renderPass_->reset();
    }

    // Mark as touched as soon as we mutate the state
    renderPass_->markAsTouched();

    /**
     *
     */
    _pipeline->invoke(renderPass_);

    // TODO:
    for (const auto& entry : renderPass_->batches()) {

        // Warning: Temporary merging of sub signals into root batch
        for (const auto& signal : entry.signals()) {
            batch_.pushSignal(signal);
        }

        for (u32 bidx { 0ui32 }; bidx < entry.barriers().size(); ++bidx) {
            batch_.pushBarrier(entry.barriers()[bidx], batch_.barrierStages()[bidx]);
        }

        for (const auto& buffer : entry.buffers()) {
            batch_.push(buffer);
        }
    }

    #if defined(_DEBUG) || TRUE
    if (batch_.buffers().empty()) {

        vk::SubmitInfo info {
            static_cast<u32>(batch_.barriers().size()),
            batch_.barriers().data(),
            batch_.barrierStages().data(),
            0,
            nullptr,
            static_cast<u32>(batch_.signals().size()),
            batch_.signals().data()
        };

        vk::Fence fence { renderPass_->unsafeSync() };
        if (!fence) {
            fence = { _device->vkDevice().createFence(vk::FenceCreateInfo {}) };
            #ifdef _DEBUG
            assert(renderPass_->storeSync(vk::Fence { fence }));
            #else
            renderPass_->storeSync(_STD move(fence));
            #endif

        } else {
            _device->vkDevice().resetFences(1, &fence);
        }

        auto* queue { _device->graphicsQueue() };
        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, fence) };
        return;
    }
    #endif

    /**
     * Warning: Temporary Enforced Submission Order
     */
    #ifdef _DEBUG
    assert(!batch_.buffers().empty());
    #endif

    const auto requiredSignals { batch_.buffers().size() - 1ui32 };
    if (requiredSignals > renderPass_->lastSignals().size()) {
        for (u64 i = renderPass_->lastSignals().size(); i < requiredSignals; ++i) {
            renderPass_->lastSignals().push_back(
                _device->vkDevice().createSemaphore(
                    {
                        vk::SemaphoreCreateFlags {}
                    }
                )
            );
        }
    }

    /**
     *
     */
    vk::Fence fence { renderPass_->unsafeSync() };
    if (!fence) {
        fence = { _device->vkDevice().createFence(vk::FenceCreateInfo {}) };
        #ifdef _DEBUG
        assert(renderPass_->storeSync(vk::Fence { fence }));
        #else
        renderPass_->storeSync(_STD move(fence));
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
        batch_.barrierStages().data(),
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
        info.pSignalSemaphores = &renderPass_->lastSignals()[0];
        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, vk::Fence {}) };
    }

    // Dynamic Enforced
    for (u32 i = 1; i < requiredSignals; ++i) {

        info.waitSemaphoreCount = 1ui32;
        info.pWaitSemaphores = &renderPass_->lastSignals()[i - 1ui32];

        info.pCommandBuffers = &batch_.buffers()[i].vkCommandBuffer();

        info.signalSemaphoreCount = 1ui32;
        info.pSignalSemaphores = &renderPass_->lastSignals()[i];

        if (batch_.buffers()[i]._pipelineBindPoint == vk::PipelineBindPoint::eGraphics) [[likely]]
        {
            vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eColorAttachmentOutput };
            info.pWaitDstStageMask = &waitStage;

            [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, vk::Fence {}) };

        } else if (batch_.buffers()[i]._pipelineBindPoint == vk::PipelineBindPoint::eCompute) {

            vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eAllCommands };
            //vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eComputeShader };
            info.pWaitDstStageMask = &waitStage;

            [[maybe_unused]] auto res { _device->computeQueue()->vkQueue().submit(1ui32, &info, vk::Fence {}) };

        } else {
            throw _STD runtime_error("Unsupported bind point.");
        }
    }

    // Last Enforced
    if (&batch_.buffers().front() != &batch_.buffers().back()) {
        // Front != Back ~ at least two submissions
        info.waitSemaphoreCount = 1ui32;
        info.pWaitSemaphores = &renderPass_->lastSignals()[requiredSignals - 1ui32];
        vk::PipelineStageFlags waitStage { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        info.pWaitDstStageMask = &waitStage;

        info.pCommandBuffers = &batch_.buffers().back().vkCommandBuffer();

        info.signalSemaphoreCount = static_cast<u32>(batch_.signals().size());
        info.pSignalSemaphores = batch_.signals().data();

        [[maybe_unused]] auto res { queue->vkQueue().submit(1ui32, &info, fence) };
    }
}

const non_owning_rptr<HORenderPass> Renderer::invoke(const non_owning_rptr<HORenderPass> renderPass_) {
    /**
     * Forward Invocation with default CommandBatch
     */
    this->invokeBatched(renderPass_, {});
    return renderPass_;
}

const non_owning_rptr<HORenderPass> Renderer::invoke(
    const non_owning_rptr<HORenderPass> renderPass_,
    cref<CommandBatch> batchLayout_
) {

    #ifdef _DEBUG
    // Ensure batch layout does not contain any command buffers
    assert(batchLayout_.buffers().empty());
    #endif

    /**
     * Forward Invocation with custom CommandBatch
     */
    this->invokeBatched(renderPass_, CommandBatch { batchLayout_ });
    return renderPass_;
}

const non_owning_rptr<const RenderPipeline> Renderer::pipeline() const noexcept {
    return _pipeline;
}

const non_owning_rptr<RenderPipeline> Renderer::getOrCreatePipeline() {
    if (_pipeline == nullptr) {
        _pipeline = new RenderPipeline();
    }

    return _pipeline;
}
