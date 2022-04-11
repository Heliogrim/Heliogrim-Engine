#pragma once

#include <Engine.Session/Session.hpp>

#include "Application/Application.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Renderer/__fwd.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

/**
 * Forward Declaration
 */
namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx {
    class Camera;
}

namespace ember::engine::gfx::loader {
    class GeometryLoader;
    class TextureLoader;
}

namespace ember::engine {

    class Graphics {
    public:
        using this_type = Graphics;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param session_ (Optional) The session.
         */
        Graphics(cref<sptr<Session>> session_ = Session::get()) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Graphics();

    public:
        /**
         * Setups this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void setup();

        /**
         * Schedules this module
         *
         * @author Julius
         * @date 19.12.2020
         */
        void schedule();

    private:
        /**
         * Tidy this up
         *
         * @author Julius
         * @date 13.09.2021
         */
        void tidy();

    private:
        /**
         * The session this module is associated with
         */
        sptr<Session> _session;

    public:
        /**
         * Gets the session this module is associated with
         *
         * @author Julius
         * @date 14.10.2021
         *
         * @returns A ptr<Session>
         */
        [[nodiscard]] sptr<Session> session() const noexcept;

    private:
        /**
         * The internal state of scheduler
         */
        _STD atomic_uint8_t _scheduled { 0 };

    private:
        gfx::Application _application;
        gfx::Surface _surface;

    private:
        /**
         * Device
         */
        sptr<gfx::Device> _device;

    public:
        /**
         * Gets current device
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns The current device.
         */
        [[nodiscard]] sptr<gfx::Device> getCurrentDevice() const noexcept;

    private:
        /**
         * Swapchain
         */
        ptr<gfx::Swapchain> _swapchain;

    public:
        /**
         * Gets current swapchain
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns The current swapchain.
         */
        [[nodiscard]] ptr<gfx::Swapchain> getCurrentSwapchain() const noexcept;

    private:
        /**
         * Compute Queue
         */
        ptr<gfx::CommandQueue> _computeQueue;

    public:
        /**
         * Gets compute queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The compute queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getComputeQueue() const noexcept;

    private:
        /**
         * Graphics
         */
        ptr<gfx::CommandQueue> _graphicsQueue;

    public:
        /**
         * Gets graphics queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The graphics queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getGraphicsQueue() const noexcept;

    private:
        /**
         * Transfer Queue
         */
        ptr<gfx::CommandQueue> _transferQueue;

    public:
        /**
         * Gets transfer queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The transfer queue.
         */
        [[nodiscard]] gfx::CommandQueue::reference_type getTransferQueue() const noexcept;

    private:
        ptr<gfx::render::Renderer> _renderer;

    private:
        Vector<ptr<gfx::render::HORenderPass>> _renderPasses;

        // Warning: Temporary
        ptr<gfx::Camera> _camera;
        Vector<gfx::CommandBatch> _renderPassBatches;

        #if FALSE
    private:
        /**
         * Graphic Passes
         */
        Vector<ptr<gfx::GraphicPass>> _graphicPasses;

        /**
         * Graphic Pass Fences[GraphicPassMask:: ...]
         */
        Vector<vk::Fence> _graphicPassFences;

        /**
         * Graphic Pass Batches
         */
        Vector<gfx::CommandBatch> _graphicPassBatches;

    public:
        /**
         * Gets a Graphic pass by given mask
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param  mask_ The mask.
         *
         * @returns A ptr&lt;gfx::GraphicPass&gt;
         */
        [[nodiscard]] ptr<gfx::GraphicPass> graphicPass(gfx::GraphicPassMask mask_) const noexcept;
        #endif

    private:
        struct {
            Vector<vk::Semaphore> image;
            Vector<vk::Semaphore> finish;
            Vector<vk::Fence> cpuGpuSync;
        } _onFlightSync;

        /**
         * Internal tick() function to invoke graphics
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @param scene_ The scene to render.
         */
        void _tick(ptr<scene::IRenderScene> scene_);

        /**
         * Process the graphic passes
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param scene_ The scene to render.
         */
        void processGraphicPasses(ptr<scene::IRenderScene> scene_);

    private:
        void reschedule();

    private:
        ptr<scene::IRenderScene> _renderScene;

    public:
        bool useAsRenderScene(const ptr<scene::IRenderScene> scene_);

    private:
        ptr<scene::IRenderScene> _uiRenderScene;

    public:
        bool useAsUIRenderScene(const ptr<scene::IRenderScene> scene_);

    private:
        Vector<ptr<gfx::loader::GeometryLoader>> _geometryLoader;
        Vector<ptr<gfx::loader::TextureLoader>> _textureLoader;

    public:
        void registerLoader();

    private:
        Vector<ptr<void>> _importer;

    public:
        void registerImporter();
    };
}
