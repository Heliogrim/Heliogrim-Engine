#pragma once

#include <Engine.Scheduler/Fiber/Awaitable.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "HORenderPassCreateData.hpp"
#include "__fwd.hpp"
#include "../vkinc.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class CommandBatch;
    class Texture;
}

namespace ember::engine::gfx::scene {
    class SceneView;
}

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx::render {
    class HORenderPass {
    public:
        using this_type = HORenderPass;

        using RenderScene = ::ember::engine::scene::IRenderScene;

    public:
        HORenderPass(
            const non_owning_rptr<Renderer> renderer_,
            HORenderPassCreateData data_,
            cref<sptr<RenderPassState>> state_
        );

        HORenderPass(cref<this_type>) = delete;

        HORenderPass(mref<this_type> other_) noexcept;

        ~HORenderPass();

    private:
        void tidy();

    private:
        _STD atomic_flag _reset;

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

    private:
        ptr<RenderScene> _scene;

    public:
        [[nodiscard]] const ptr<RenderScene> scene() const noexcept;

        bool use(const ptr<RenderScene> scene_) noexcept;

    private:
        ptr<scene::SceneView> _sceneView;

    public:
        [[nodiscard]] const ptr<scene::SceneView> sceneView() const noexcept;

        bool use(const ptr<scene::SceneView> sceneView_) noexcept;

    private:
        sptr<Texture> _target;

    public:
        [[nodiscard]] cref<sptr<Texture>> target() const noexcept;

    private:
        non_owning_rptr<Renderer> _renderer;

    public:
        [[nodiscard]] const non_owning_rptr<Renderer> renderer() const noexcept;

    private:
        sptr<RenderPassState> _state;

    public:
        [[nodiscard]] cref<sptr<RenderPassState>> state() const noexcept;

        [[nodiscard]] sptr<RenderPassState> state() noexcept;

        /**
         * TODO: Refactor to cleanup HORenderPass Interface
         */
    private:
        Vector<CommandBatch> _batches;

    public:
        [[nodiscard]] ref<decltype(_batches)> batches() noexcept;

    private:
        Vector<vk::Semaphore> _lastSignals;

    public:
        [[nodiscard]] ref<decltype(_lastSignals)> lastSignals() noexcept;

    private:
        _STD atomic_ptrdiff_t _sync;

    private:
        void clearSync();

    public:
        [[nodiscard]] bool storeSync(mref<vk::Fence> fence_);

        [[nodiscard]] vk::Fence unsafeSync() const noexcept;

        [[nodiscard]] bool await() const noexcept;
    };

    static_assert(::ember::engine::scheduler::fiber::IsAwaitable<HORenderPass>);
}
