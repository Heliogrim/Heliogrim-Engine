#pragma once

#include <atomic>
#include "../../Types.hpp"
#include "../../Collection/Vector.hpp"

namespace hg::engine::concurrent {
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
	template <class Ty, class Allocator = std::allocator<Ty>, class ContainerType = hg::Vector<Ty, Allocator>>
	class RingBuffer {
	public:
		using size_type = typename ContainerType::size_type;

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
		 * @param  capacity_ (Optional) The fixed capacity of contained elements `[0..N(`.
		 */
		RingBuffer(size_type capacity_ = 0x20uL) :
			_container(capacity_),
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
			const size_type h = _head.load(std::memory_order::acquire);
			const size_type n = inc(h);

			if (n != _tail.load(std::memory_order_consume)) {
				_container[n] = value_;
				_head.store(n, std::memory_order::release);
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
			const size_type t = _tail.load(std::memory_order_consume);
			const size_type h = _head.load(std::memory_order_consume);
			const size_type n = inc(t);

			/**
			 * Head [7] > Tail [2] => Pass
			 * Head [8] = Tail [8] => Fail
			 * Head [3] < Tail [27] => Fail
			 * Head [3] < Tail [27] && n[28] > Tail [27] => Pass
			 * Head [3] < Tail [31] && Head [3] >= n[0] => Pass
			 */
			if (h > t || (h < t && (h >= n || n > t))) {
				value_ = std::move(_container[n]);
				_tail.store(n, std::memory_order::release);
				return true;
			}

			return false;
		}

		/**
		 * Check whether this buffer is empty
		 *
		 * @author Julius
		 * @date 15.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool empty() const {
			const size_t h = _head.load(std::memory_order::consume), t = _tail.load(std::memory_order::consume);
			return (h - t) == 0;
		}

		/**
		 * Check whether this buffer is full
		 *
		 * @author Julius
		 * @date 15.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool full() const {
			const size_type h = _head.load(std::memory_order_consume);
			return inc(h) == _tail.load(std::memory_order_consume);
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

		alignas(CACHE_LINE_SIZE) std::atomic<size_t> _head;
		alignas(CACHE_LINE_SIZE) std::atomic<size_t> _tail;

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
		[[nodiscard]] size_type inc(size_t value_) const noexcept {
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
		[[nodiscard]] size_type idx(size_t value_) noexcept {
			return value_ % reserved();
		}
	};
}
