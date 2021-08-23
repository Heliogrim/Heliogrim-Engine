#pragma once

#include <atomic>
#include "../../Types.hpp"
#include "../../Collection/List.hpp"

namespace ember::engine::concurrent {

    /**
     * A Single-Producer, Single-Consumer - Ring Buffer
     *
     * @author Julius
     * @date 15.11.2020
     *
     * @tparam Ty The value type for the container.
     * @tparam Allocator Type of the allocator.
     * @tparam ContainerType Type of the container type.
     */
    template <class Ty, class Allocator = _STD allocator<Ty>, class ContainerType = ember::vector<Ty, Allocator>>
    class RingBuffer {
    public:
        using size_type = u32;

        using value_type = Ty;
        using reference_type = Ty&;
        using const_reference_type = const Ty&;

        using container_type = ContainerType;

        using this_type = RingBuffer<Ty>;

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  reserved_ (Optional) The reserved.
         */
        RingBuffer(size_type reserved_ = 0x20ui32) :
            _container(reserved_),
            _head(0),
            _tail(0) {}

        /**
         * Attempts to push
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param [in,out] value_ The value.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool try_push(Ty&& value_) {
            const size_type h = _head.load(_STD memory_order::seq_cst);
            const size_type n = inc(h);

            if (n != _tail.load()) {
                _container[n] = value_;
                _head.store(n, std::memory_order::seq_cst);
                return true;
            }

            return false;
        }

        /**
         * Attempts to pop
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param [in,out] value_ The value.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool try_pop(Ty& value_) {
            const size_type t = _tail.load(_STD memory_order::seq_cst);
            const size_type h = _head.load(_STD memory_order::seq_cst);
            const size_type n = inc(t);

            if (h > t || (h >= n && h < t)) {
                value_ = _STD move(_container[n]);
                _tail.store(n, _STD memory_order::seq_cst);
                return true;
            }

            return false;
        }

        /**
         * Empties this
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const {
            const size_t h = _head.load(_STD memory_order::seq_cst), t = _tail.load(_STD memory_order::seq_cst);
            return (h - t) == 0;
        }

        /**
         * Fulls this
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool full() const {
            const size_type h = _head.load(_STD memory_order::seq_cst), t = _tail.load(_STD memory_order::seq_cst);
            return (h - t) != 0 && (h + t) % (reserved() - 1) == 0;
        }

        /**
         * Gets the reserved
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type reserved() const {
            return _container.capacity();
        }

        /**
         * Unsafe container
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @returns A reference to a container_type.
         */
        [[nodiscard]] container_type& unsafe_container() {
            return _container;
        }

    private:
        container_type _container;

        _STD atomic<size_type> _head;
        _STD atomic<size_type> _tail;

        /**
         * Increments the given value
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  value_ The value.
         *
         * @returns A size_type.
         */
        [[nodiscard]] FORCE_INLINE size_type inc(size_t value_) noexcept {
            return (++value_) % reserved();
        }

        /**
         * Indexes the given value
         *
         * @author Julius
         * @date 15.11.2020
         *
         * @param  value_ The value.
         *
         * @returns A size_type.
         */
        [[nodiscard]] FORCE_INLINE size_type idx(size_t value_) noexcept {
            return value_ % reserved();
        }
    };
}
