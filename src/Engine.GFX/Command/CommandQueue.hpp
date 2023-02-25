#pragma once

#include <Engine.Common/Types.hpp>
#include "../Device/Device.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration CommandPool.
     *
     * @author Julius
     * @date 09.12.2020
     */
    class CommandPool;

    /**
     * Forward Declaration CommandBuffer.
     *
     * @author Julius
     * @date 11.12.2020
     */
    class CommandBuffer;

    /**
     * Forward Declaration CommandBatch.
     *
     * @author Julius
     * @date 11.12.2020
     */
    class CommandBatch;

    class CommandQueue final {
    public:
        using value_type = CommandQueue;
        using reference_type = CommandQueue&;
        using const_reference_type = const CommandQueue&;

        /**
         * Constructor
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param  device_ The device.
         * @param  idx_ Zero-based index of the.
         * @param  familyIdx_ Zero-based index of the family.
         */
        CommandQueue(cref<sptr<Device>> device_, const u32 idx_, const u32 familyIdx_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 09.11.2020
         */
        ~CommandQueue() = default;

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
         * Cast that converts the given  to a bool
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const;

        /**
         * Submits the given batch
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @param  batch_ The batch.
         * @param  fence_ (Optional) The fence.
         */
        void submit(const CommandBatch& batch_, const vk::Fence& fence_ = nullptr);

        /**
         * Submit the given buffer and waits
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param buffer_ The buffer.
         */
        void submitWait(const CommandBuffer& buffer_);

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    public:
        /**
         * Gets the device
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] const sptr<Device>& device() const noexcept;

    private:
        /**
         * Vulkan API
         */
        u32 _idx = ~0;
        u32 _familyIdx = ~0;

        vk::Queue _vkQueue;

    public:
        /**
         * Family index
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 familyIdx() const noexcept;

        /**
         * Vk queue
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @returns A reference to a const vk::Queue.
         */
        [[nodiscard]] const vk::Queue& vkQueue() const noexcept;

    private:
        /**
         * Pools
         */
        ptr<CommandPool> _pool;

    public:
        /**
         * Gets the pool
         *
         * @author Julius
         * @date 08.12.2020
         *
         * @returns A reference to a CommandPool.
         */
        [[nodiscard]] ptr<CommandPool> pool() const noexcept;
    };
}
