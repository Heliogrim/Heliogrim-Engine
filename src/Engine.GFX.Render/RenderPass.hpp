#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderGraph/Pass/ExecutionPass.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "RenderPassResult.hpp"
#include "RenderPassState.hpp"

namespace hg::engine::gfx::render {
    class RenderPass final {
    public:
        friend class Renderer;

    public:
        using this_type = RenderPass;

    public:
        RenderPass(
            mref<nmpt<const Renderer>> renderer_,
            mref<uptr<graph::RuntimeGraph>> runtimeGraph_,
            mref<smr<cache::LocalCacheCtrl>> localGeneralCache_,
            mref<smr<memory::LocalPooledAllocator>> localDeviceAllocator_
        ) noexcept;

        RenderPass(cref<this_type>) = delete;

        RenderPass(mref<this_type>) = delete;

        ~RenderPass() noexcept;

    private:
        nmpt<const Renderer> _renderer;

    protected:
        RenderPassState _state;
        uptr<graph::RuntimeGraph> _graph;

    protected:
        bool alloc();

        bool realloc();

        bool free();

    private:
        [[nodiscard]] graph::ExecutionPass getExecutionPass() noexcept;

    public:
        RenderPassResult operator()();

    public:
        nmpt<scene::SceneView> changeSceneView(mref<nmpt<scene::SceneView>> nextSceneView_);

    public:
        #pragma region Render Pass Targets
        void unsafeBindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<nmpt<void>> resource_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<nmpt<Texture>> texture_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<nmpt<TextureView>> textureView_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<nmpt<VirtualTexture>> texture_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<nmpt<VirtualTextureView>> textureView_
        );

        _Success_(return != nullptr) nmpt<void> unbindTarget(mref<smr<const acc::Symbol>> target_) noexcept;

        #pragma endregion Render Pass Targets

    private:
        #pragma region Render Pass Synchronization
        /* Previous Synchronization State */
        _STD atomic_ptrdiff_t _sync;
        _STD atomic_flag _reset;

        // May be used to synchronize command buffer submission related to swapchain dispatching
        Vector<vk::Semaphore> _targetWaitSignals;
        Vector<vk::PipelineStageFlagBits> _targetWaitSignalStages;
        Vector<vk::Semaphore> _targetReadySignals;

    private:
        void clearSync();

    public:
        [[nodiscard]] bool storeSync(mref<vk::Fence> fence_);

        [[nodiscard]] vk::Fence unsafeSync() const noexcept;

        [[nodiscard]] bool await() const noexcept;

    public:
        [[nodiscard]] bool isReset() const noexcept;

        /**
         * Reset the internal state and signals of this HORenderPass
         *
         * @details Will internally check, whether this instance is in a pending state or was already reset. If the check fails, the call will return immediately without any side effects.
         *  If the check succeeds the state and it's caches will be reset/shifted, signal will be lifted and fences destroyed.
         */
        void reset();

        void markAsTouched();

    public:
        [[nodiscard]] ref<decltype(_targetWaitSignals)> getTargetWaitSignals() noexcept;

        [[nodiscard]] ref<decltype(_targetWaitSignalStages)> getTargetWaitSignalStages() noexcept;

        [[nodiscard]] ref<decltype(_targetReadySignals)> getTargetReadySignals() noexcept;

        #pragma endregion Render Pass Synchronization
    };
}

static_assert(::hg::engine::scheduler::fiber::IsAwaitable<hg::engine::gfx::render::RenderPass>);
