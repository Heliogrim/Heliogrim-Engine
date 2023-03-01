#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include "../vkinc.hpp"
#include "../Application/Application.hpp"
#include "../Memory/GlobalPooledAllocator.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration CommandQueue.
     *
     * @author Julius
     * @date 11.12.2020
     */
    class CommandQueue;

    /**
     * Forward Declaration Surface.
     *
     * @author Julius
     * @date 11.12.2020
     */
    class Surface;

    class Device final :
        public _STD enable_shared_from_this<Device> {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 12.12.2022
         *
         * @param application_ The application instances used for tracking and encapsulation.
         */
        Device(const Application& application_);

        /**
         * Constructor
         *
         * @author Julius
         * @date 17.11.2020
         *
         * @param  application_ The application.
         * @param  surface_ The surface.
         */
        Device(const Application& application_, ptr<Surface> surface_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~Device();

        /**
         * Setups this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void setup();

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        void destroy();

        /**
         * Gets the vk device
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns A reference to a vk::Device.
         */
        vk::Device& vkDevice() const;

        /**
         * Gets the vk physical device
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns A reference to a vk::PhysicalDevice.
         */
        vk::PhysicalDevice& vkPhysicalDevice() const;

    private:
        uptr<memory::GlobalPooledAllocator> _alloc;

    public:
        [[nodiscard]] const non_owning_rptr<memory::GlobalPooledAllocator> allocator() const noexcept;

    private:
        /**
         * Compute Queue
         */
        ptr<CommandQueue> _computeQueue;

    public:
        /**
         * Calculates the queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns The calculated queue.
         */
        ptr<CommandQueue> computeQueue() const noexcept;

    private:
        /**
         * Graphics Queue
         */
        ptr<CommandQueue> _graphicsQueue;

    public:
        /**
         * Graphics queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns A ptr&lt;CommandQueue&gt;
         */
        ptr<CommandQueue> graphicsQueue() const noexcept;

    private:
        /**
         * Transfer Queue
         */
        ptr<CommandQueue> _transferQueue;

    public:
        /**
         * Transfer queue
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns A ptr&lt;CommandQueue&gt;
         */
        ptr<CommandQueue> transferQueue() const noexcept;

    private:
        Application _application;
        ptr<Surface> _surface;

        mutable vk::Device _device;
        mutable vk::PhysicalDevice _physicalDevice;

        /**
         * Gets queue family index
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param  flag_ The flag.
         *
         * @returns The queue family index.
         */
        uint32_t getQueueFamilyIdx(const vk::QueueFlagBits& flag_);
    };
}
