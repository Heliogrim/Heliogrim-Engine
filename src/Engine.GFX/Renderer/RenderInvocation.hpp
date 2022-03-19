#pragma once

#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "../Camera/Camera.hpp"
#include "../Texture/Texture.hpp"
#include "../Command/CommandBatch.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class Renderer;
    struct RenderInvocationData;
    struct RenderInvocationState;
}

namespace ember::engine::gfx {

    class RenderInvocation {
    public:
        using this_type = RenderInvocation;

    public:
        RenderInvocation(const non_owning_rptr<Renderer> renderer_, RenderInvocationData data_,
            const sptr<RenderInvocationState> state_);

        RenderInvocation(cref<this_type>) = delete;

        // Warning: explicitly defaulted move constructor is implicitly deleted [-Wdefaulted-function-deleted]	Ember	R:\Development\C++\Vulkan API\Game\src\Engine.GFX\Renderer\RenderInvocation.hpp	31
        RenderInvocation(mref<this_type> other_) noexcept = default;

        ~RenderInvocation();

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
        sptr<RenderInvocationState> _state;

    public:
        [[nodiscard]] cref<sptr<RenderInvocationState>> state() const noexcept;

        /**
         * TODO: Refactor to cleanup RenderInvocation Interface
         */
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

    static_assert(scheduler::fiber::IsAwaitable<RenderInvocation>);
}
