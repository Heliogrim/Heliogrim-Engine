#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "RenderEnqueueResult.hpp"
#include "vkinc.hpp"

namespace hg::engine::gfx::render {
    class HORenderPass;
    class Renderer;
}

namespace hg::engine::gfx {
    class Camera;
    class Device;
    class Surface;
    class Swapchain;
}

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::gfx {
    class RenderTarget {
    public:
        using this_type = RenderTarget;

    public:
        RenderTarget();

        ~RenderTarget();

    private:
        void tidy();

    private:
        /**
         * External device bound to this render target
         *
         * @warning Every sub-component of this render target must be compatible or derived from bound device
         */
        sptr<Device> _device;

    public:
        sptr<Device> use(cref<sptr<Device>> device_);

        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         * External renderer used to render to this target
         */
        non_owning_rptr<render::Renderer> _renderer;
        Vector<ptr<render::HORenderPass>> _passes;

    public:
        non_owning_rptr<render::Renderer> use(cref<non_owning_rptr<render::Renderer>> renderer_);

    private:
        /**
         * External swapchain used to control next rendering and asynchronous dispatches
         *
         * @warning If used in combination with surface, Swapchain must be compatible or derived by Surface
         */
        non_owning_rptr<Swapchain> _swapchain;

    public:
        non_owning_rptr<Swapchain> use(cref<non_owning_rptr<Swapchain>> swapchain_);

        [[nodiscard]] u32 supportedFramesAhead() const noexcept;

    private:
        /**
         * External surface used to present/display results
         *
         * @see _swapchain
         */
        non_owning_rptr<Surface> _surface;

    public:
        non_owning_rptr<Surface> use(cref<non_owning_rptr<Surface>> surface_);

    private:
        /**
         * Internal semaphores for synchronization
         */
        Vector<vk::Semaphore> _otfImage;
        Vector<vk::Semaphore> _otfFinish;

    private:
        /**
         * Internal fences for synchronization
         */
        Vector<vk::Fence> _otfCpuGpu;

    private:
        /**
         * Internal attributes for behaviour modification
         */
        bool _enforceCpuGpuSync;
        bool _onTheFlight;
        u32 _syncIdx;

    private:
        u32 _swapIdx;
        vk::Semaphore _swapSignal;

    private:
        void nextSync();

    private:
        /*_STD atomic_flag*/
        bool _active;

    public:
        /**
         *
         */
        [[nodiscard]] bool ready() const noexcept;

        [[nodiscard]] bool active() const noexcept;

        /**
         * Set this render target active or inactive
         *
         * @param active_ Flag to set
         *
         * @returns True when active state has changed, otherwise false.
         */
        bool setActive(const bool active_);

    public:
        /**
         *
         * @blocking
         */
        void buildPasses(const ptr<scene::SceneView> sceneView_);

        /**
         *
         * @blocking
         */
        [[nodiscard]] bool rebuildPasses(cref<non_owning_rptr<Swapchain>> swapchain_);

    public:
        /**
         *
         * @blocking
         */
        const non_owning_rptr<render::HORenderPass> next();

        /**
         *
         * @blocking
         */
        void update();

        /**
         *
         * @non-blocking
         */
        RenderEnqueueResult finish(cref<Vector<vk::Semaphore>> waits_/* , cref<Vector<vk::Semaphore>> signals_ */);
    };
}
