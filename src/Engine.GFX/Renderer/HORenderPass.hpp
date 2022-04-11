#pragma once

#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "HORenderPassCreateData.hpp"
#include "__fwd.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class Camera;
    class Texture;
    class CommandBatch;
}

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx::render {

    class HORenderPass {
    public:
        using this_type = HORenderPass;

    public:
        HORenderPass(const non_owning_rptr<Renderer> renderer_, HORenderPassCreateData data_,
            cref<sptr<RenderPassState>> state_);

        HORenderPass(cref<this_type>) = delete;

        HORenderPass(mref<this_type> other_) noexcept;

        ~HORenderPass();

    private:
        void tidy();

    private:
        ptr<scene::IRenderScene> _scene;

    public:
        [[nodiscard]] const ptr<scene::IRenderScene> scene() const noexcept;

        bool use(const ptr<scene::IRenderScene> scene_) noexcept;

    private:
        ptr<Camera> _camera;

    public:
        [[nodiscard]] const ptr<Camera> camera() const noexcept;

        bool use(const ptr<Camera> camera_) noexcept;

    private:
        ptr<Texture> _target;

    public:
        [[nodiscard]] const ptr<Texture> target() const noexcept;

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

    public:
        [[nodiscard]] bool storeSync(mref<vk::Fence> fence_);

        [[nodiscard]] vk::Fence unsafeSync() const noexcept;

        [[nodiscard]] bool await() const noexcept;
    };

    static_assert(scheduler::fiber::IsAwaitable<HORenderPass>);
}
