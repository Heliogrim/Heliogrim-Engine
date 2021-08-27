#pragma once

#include "GraphicPass.hpp"
#include "Application/Application.hpp"
#include "Command/CommandQueue.hpp"
#include "Device/Device.hpp"
#include "Surface/Surface.hpp"
#include "Swapchain/Swapchain.hpp"

namespace ember::engine {

    class Graphics {
    public:

        using value_type = Graphics;
        using reference_type = Graphics&;
        using const_reference_type = const Graphics&;

    private:
        /**
         * Singleton Instance
         */
        static ptr<Graphics> _instance;

    public:

        /**
         * Gets static store instance
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns A ptr&lt;Graphics&gt;
         */
        [[nodiscard]] static ptr<Graphics> get() noexcept;

        /**
         * Gets or create static stored instance
         *
         * @author Julius
         * @date 29.01.2021
         *
         * @returns A ptr&lt;Graphics&gt;
         */
        [[nodiscard]] static ptr<Graphics> make();

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        Graphics() = default;

    public:

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Graphics();

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

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 10.11.2020
         */
        void destroy();

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
        /**
         * Graphic Scene Graph
         */
        ptr<scene::SceneGraph> _graph;

    private:
        /**
         * Graphic Passes
         */
        vector<ptr<gfx::GraphicPass>> _graphicPasses;

        /**
         * Graphic Pass Fences[GraphicPassMask:: ...]
         */
        vector<vk::Fence> _graphicPassFences;

        /**
         * Graphic Pass Batches
         */
        vector<gfx::CommandBatch> _graphicPassBatches;

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

    private:
        struct {
            vector<vk::Semaphore> image;
            vector<vk::Semaphore> finish;
            vector<vk::Fence> cpuGpuSync;
        } _onFlightSync;

        /**
         * Internal tick() function to invoke graphics
         *
         * @author Julius
         * @date 13.12.2020
         */
        void _tick();

        /**
         * Process the graphic passes
         *
         * @author Julius
         * @date 31.01.2021
         */
        void processGraphicPasses();
    };
}
