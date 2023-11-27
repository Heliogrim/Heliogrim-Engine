#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderGraph/Pass/ExecutionPass.hpp>
#include <Engine.GFX/vkinc.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>
#include <Engine.GFX/Texture/__fwd.hpp>

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
    public:
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
        smr<const scene::SceneView> changeSceneView(mref<smr<const scene::SceneView>> nextSceneView_);

        smr<const scene::SceneView> unbindSceneView();

    public:
        #pragma region Render Pass Targets
        void unsafeBindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<smr<void>> resource_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<smr<Texture>> texture_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<smr<TextureView>> textureView_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<smr<VirtualTexture>> texture_
        );

        [[nodiscard]] bool bindTarget(
            mref<smr<const acc::Symbol>> target_,
            mref<smr<VirtualTextureView>> textureView_
        );

        _Success_(return != nullptr) smr<void> unbindTarget(mref<smr<const acc::Symbol>> target_) noexcept;

        #pragma endregion Render Pass Targets

    private:
        #pragma region Render Pass Synchronization
        /* Previous Synchronization State */
        _STD atomic_ptrdiff_t _sync;
        _STD atomic_flag _reset;

        // May be used to synchronize command buffer submission related to swapchain dispatching
        Vector<vk::Semaphore> _targetWaitSignals;

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
        bool addTargetWaitSignal(mref<smr<const acc::Symbol>> targetSymbol_, cref<vk::Semaphore> signal_) noexcept;

        void clearTargetWaitSignals(mref<smr<const acc::Symbol>> targetSymbol_) noexcept;

        void enumerateTargetWaitSignals(
            ref<Vector<vk::Semaphore>> signals_,
            _In_opt_ mref<smr<const acc::Symbol>> targetSymbol_ = nullptr
        ) noexcept;

        void enumerateTargetReadySignals(
            ref<Vector<vk::Semaphore>> signals_,
            _In_opt_ mref<smr<const acc::Symbol>> targetSymbol_ = nullptr
        ) noexcept;

        #pragma endregion Render Pass Synchronization
    };
}

static_assert(::hg::engine::scheduler::fiber::IsAwaitable<hg::engine::gfx::render::RenderPass>);
