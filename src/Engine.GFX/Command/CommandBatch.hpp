#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include "CommandBuffer.hpp"
#include "../vkinc.hpp"

namespace ember::engine::gfx {

    class CommandBatch {
    public:
        using this_type = CommandBatch;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 22.11.2020
         */
        CommandBatch() = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 25.11.2020
         *
         * @param  buffer_ The buffer.
         */
        CommandBatch(const CommandBuffer& buffer_);

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 07.03.2022
         *
         * @param other_ The other CommandBatch to copy from.
         */
        CommandBatch(cref<this_type> other_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 22.11.2020
         */
        ~CommandBatch() noexcept = default;

        /**
         * Check whether this is empty
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will check for no buffers and signals. Barriers without payload should not cause any (side-)effect.
         *
         * @returns True if no buffers and signals are present, otherwise false.
         */
        [[nodiscard]] bool empty() const noexcept;

        /**
         * Resets this
         *
         * @author Julius
         * @date 14.02.2021
         */
        void reset();

    private:
        /**
         * Buffers
         */
        Vector<CommandBuffer> _buffers;

    public:
        /**
         * Gets the buffers
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<CommandBuffer>& buffers() const noexcept;

        /**
         * Pushes an object onto this fifo
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @param [in,out] buffer_ The buffer to push.
         */
        void push(CommandBuffer&& buffer_);

        /**
         * Pushes an object onto this fifo
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @param  buffer_ The buffer to push.
         */
        void push(const CommandBuffer& buffer_);

    private:
        /**
         * Barriers ( Fence, Signal )
         */
        Vector<vk::Semaphore> _barriers;

    public:
        /**
         * Gets the barriers
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<vk::Semaphore>& barriers() const noexcept;

        /**
         * Pushes a barrier
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @param  barrier_ The semaphore.
         */
        void pushBarrier(const vk::Semaphore& barrier_);

    private:
        /**
         * Barrier Stages
         */
        vk::PipelineStageFlags _barrierStages;

    public:
        /**
         * Barrier stages
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @returns A reference to the vk::PipelineStageFlags.
         */
        [[nodiscard]] const vk::PipelineStageFlags& barrierStages() const noexcept;

        /**
         * Barrier stages
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @returns A reference to the vk::PipelineStageFlags.
         */
        [[nodiscard]] vk::PipelineStageFlags& barrierStages() noexcept;

    private:
        /**
         * Signals ( Fence, Signal )
         */
        Vector<vk::Semaphore> _signals;

    public:
        /**
         * Gets the signals
         *
         * @author Julius
         * @date 22.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<vk::Semaphore>& signals() const noexcept;

        /**
         * Pushes a signal
         *
         * @author Julius
         * @date 13.12.2020
         *
         * @param  signal_ The signal.
         */
        void pushSignal(const vk::Semaphore& signal_);
    };
}
