#pragma once

#include <Engine.Common/Concurrent/Collection/RingBuffer.hpp>

#include "Fiber.hpp"

namespace hg::engine::scheduler::fiber {
    class FiberPool {
    public:
        using value_type = FiberPool;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using pool_container_type = concurrent::RingBuffer<ptr<Fiber>>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.10.2021
         *
         * @param  reserved_ (Optional) The reserved amount of slots.
         *                   If value of reserved_ equals null, fiber pool will determine pool size by itself
         */
        FiberPool(_In_opt_ pool_container_type::size_type reserved_ = 0uL);

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 21.10.2021
         */
        FiberPool(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 21.10.2021
         */
        FiberPool(mref<value_type>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.10.2021
         */
        ~FiberPool();

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 21.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 21.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<FiberPool>) noexcept = delete;

    public:
        /**
         * Tidies this
         *
         * @author Julius
         * @date 22.10.2021
         */
        void tidy();

    private:
        std::atomic_flag _acqMtx;
        std::atomic_flag _relMtx;
        concurrent::RingBuffer<ptr<Fiber>> _pool;

    public:
        /**
         * Gets the capacity
         *
         * @author Julius
         * @date 22.10.2021
         *
         * @returns A pool_container_type::size_type.
         */
        [[nodiscard]] pool_container_type::size_type capacity() const noexcept;

    public:
        [[nodiscard]] ptr<Fiber> acquire();

        void release(_In_ mref<ptr<Fiber>> fiber_);

        void release(_In_ ref<ptr<Fiber>> fiber_);

    protected:
        [[nodiscard]] _Success_(return != nullptr) ptr<Fiber> acquireNew() noexcept;

        [[nodiscard]] _Success_(return != nullptr) ptr<Fiber> acquireReuse() noexcept;

    protected:
        [[nodiscard]] _Success_(return != false) bool restore(_Out_ ptr<Fiber> fiber_) noexcept;
    };
}
