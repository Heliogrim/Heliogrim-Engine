#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SpinLock.hpp>
#include "CommandQueue.hpp"

namespace ember::engine::gfx {

    /**
     * Forward Declaration CommandBuffer.
     *
     * @author Julius
     * @date 25.11.2020
     */
    class CommandBuffer;

    class CommandPool final {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param  queue_ The queue.
         */
        CommandPool(const ptr<CommandQueue>& queue_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.11.2020
         */
        ~CommandPool() noexcept = default;

        /**
         * Setups this
         *
         * @author Julius
         * @date 25.11.2020
         */
        void setup();

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 25.11.2020
         */
        void destroy();

        /**
         * Constructs a new CommandBuffer
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @returns A CommandBuffer.
         */
        [[nodiscard]] CommandBuffer make();

        /**
         * Releases the given buffer
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @param  buffer_ The buffer.
         */
        void release(const CommandBuffer& buffer_);

    private:
        /**
         * Command Queue
         */
        ptr<CommandQueue> _queue;

    public:
        /**
         * Gets the queue
         *
         * @author Julius
         * @date 09.12.2020
         *
         * @returns A ptr&lt;CommandQueue&gt;
         */
        [[nodiscard]] ptr<CommandQueue> queue() noexcept;

    private:
        /**
         * Synchronization Lock
         */
        mutable ember::concurrent::UnfairSpinLock _lck;

    public:
        /**
         * Get a mutable reference to internal synchronization lock
         *
         * @author Julius
         * @date 25.11.2020
         */
        [[nodiscard]] ref<ember::concurrent::UnfairSpinLock> lck() const;

    private:
        /**
         * Vulkan API
         */
        vk::CommandPool _vkPool;
    };
}
