#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>

#include "../Wrapper.hpp"
#include "../__macro.hpp"
#include "../Memory/MemoryPointer.hpp"

namespace hg::concurrent {
	namespace {
		/**
		 * The internal state of the future
		 *
		 * @author Julius
		 * @date 20.08.2020
		 */
		template <typename Ty>
		class future_state {
		public:
			future_state() :
				_returned(),
				_value(std::move(allocate())) {}

			/**
			 * Destructor
			 *
			 * @author Julius
			 * @date 20.08.2020
			 */
			~future_state() {
				_mtx.lock();

				if (_returned.test(std::memory_order::relaxed)) {
					_value->~Ty();
				}
				deallocate(_value);

				_mtx.unlock();
			}

			/**
			 * Set the returned state and notify all waiters
			 *
			 * @exception std::Thrown when a Standard error condition occurs.
			 */
			void complete() const {
				if (_returned.test_and_set(std::memory_order::acq_rel))
					throw std::runtime_error("Try to complete a already returned future state.");
				_cv.notify_all();
			}

			/**
			 * Set the internal value
			 *
			 * @exception std::Thrown when a Standard error condition occurs.
			 *
			 * @param  val_ The value.
			 */
			template <typename Type_ = Ty, typename = std::enable_if_t<std::is_nothrow_default_constructible_v<Type_> ||
				std::is_nothrow_move_constructible_v<Type_>>>
			void set(Type_&& val_) {
				if (_returned.test_and_set(std::memory_order::acq_rel))
					throw std::runtime_error("Try to assign value to already assigned future state.");

				new(_value) Ty(std::forward<Type_>(val_));
				_cv.notify_all();
			}

			/**
			 * Returns a reference to the allocated memory
			 *
			 * @returns A reference to a Ty*.
			 */
			Ty*& mem() const noexcept {
				return _value;
			}

			/**
			 * Flag whether the state represents a returned value
			 *
			 * @returns True if it succeeds, false if it fails.
			 */
			bool returned() const {
				return _returned.test(std::memory_order::relaxed);
			}

			/**
			 * Returns the internal stored value and invalidates it
			 *
			 * @returns A reference to a Ty&amp;
			 */
			Ty&& value() const {
				return std::forward<Ty&&>(*_value);
			}

			/** Waits this  */
			void wait() const {
				if (_returned.test(std::memory_order::acquire))
					return;
				std::unique_lock<std::mutex> lck(_mtx);
				_cv.wait(lck);
			}

		public:
			[[nodiscard]] non_owning_rptr<std::atomic_flag> signal() const {
				return &_returned;
			}

		private:
			mutable std::condition_variable _cv;
			mutable std::mutex _mtx;
			mutable std::atomic_flag _returned;
			mutable Ty* _value;

			/**
			 * Allocates memory for the object to store
			 *
			 * @returns Null if it fails, else a pointer to a Ty.
			 */
			static constexpr Ty* allocate() {
				return static_cast<Ty*>(malloc(sizeof(Ty)));
			}

			/**
			 * Deallocates the given address
			 *
			 * @param [in,out] addr_ If non-null, the address.
			 */
			static constexpr void deallocate(Ty*& addr_) {
				return free(addr_);
			}
		};

		template <>
		class future_state<void> {
		public:
			[[maybe_unused]] future_state() :
				_returned() {}

			/**
			 * Destructor
			 *
			 * @author Julius
			 * @date 20.08.2020
			 */
			[[maybe_unused]] ~future_state() = default;

			/**
			 * Set the returned state and notify all waiters
			 *
			 * @exception std::Thrown when a Standard error condition occurs.
			 */
			[[maybe_unused]] void complete() const {
				if (_returned.test_and_set(std::memory_order::acq_rel))
					throw std::runtime_error("Try to complete a already returned future state.");
				_cv.notify_all();
			}

			/**
			 * Set the internal value
			 *
			 * @exception std::Thrown when a Standard error condition occurs.
			 */
			[[maybe_unused]] void set() {
				if (_returned.test_and_set(std::memory_order::acq_rel))
					throw std::runtime_error("Try to assign value to already assigned future state.");
				_cv.notify_all();
			}

			/**
			 * Flag whether the state represents a returned value
			 *
			 * @returns True if it succeeds, false if it fails.
			 */
			bool returned() const {
				return _returned.test(std::memory_order::relaxed);
			}

			/** Waits this  */
			void wait() const {
				if (_returned.test(std::memory_order::acquire))
					return;
				std::unique_lock<std::mutex> lck(_mtx);
				_cv.wait(lck);
			}

		public:
			[[nodiscard]] nmpt<const std::atomic_flag> signal() const {
				return &_returned;
			}

		private:
			mutable std::condition_variable _cv;
			mutable std::mutex _mtx;
			mutable std::atomic_flag _returned;
		};
	}

	/**
	 * Future
	 */
	template <typename Ty>
	class future {
	public:
		using state_type = std::shared_ptr<future_state<Ty>>;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @param  state_ The state.
		 */
		future(const state_type& state_) :
			_state(state_) {}

		/**
		 * Check if future has returned
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool ready() const {
			return _state->returned();
		}

		/**
		 * Returns value of the future and invalidates the state
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @returns A reference to a Ty&amp;
		 */
		Ty&& retrieve() const {
			return std::move(_state->value());
		}

		/**
		 * Will wait until the future returned, returns the value and invalidates the state
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @returns A reference to a Ty&amp;
		 */
		Ty&& get() const {
			if (!_state->returned())
				_state->wait();
			return std::move(_state->value());
		}

		/**
		 * Will wait until the future returned
		 *
		 * @author Julius
		 * @date 20.08.2020
		 */
		void join() const {
			_state->wait();
		}

		/**
		 * Will return a reference to the internal state of the future, which should not be mutated
		 *
		 * @author Julius
		 * @date 22.08.2020
		 */
		[[maybe_unused]] const state_type& state() const {
			return _state;
		}

	public:
		[[nodiscard]] nmpt<const std::atomic_flag> signal() const {
			return _state->signal();
		}

	private:
		state_type _state;
	};

	template <>
	class future<void> {
	public:
		using state_type = std::shared_ptr<future_state<void>>;

		/**
		 * Constructor
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @param  state_ The state.
		 */
		[[maybe_unused]] future(const state_type& state_) :
			_state(state_) {}

		/**
		 * Check if future has returned
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool ready() const {
			return _state->returned();
		}

		/**
		 * Returns value of the future and invalidates the state
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 */
		[[maybe_unused, noreturn]] void retrieve() const noexcept {
			std::unreachable();
		}

		/**
		 * Will wait until the future returned, returns the value and invalidates the state
		 *
		 * @author Julius
		 * @date 20.08.2020
		 *
		 */
		void get() const {
			if (!_state->returned())
				_state->wait();
		}

		/**
		 * Will wait until the future returned
		 *
		 * @author Julius
		 * @date 20.08.2020
		 */
		void join() const {
			_state->wait();
		}

		/**
		 * Will return a reference to the internal state of the future, which should not be mutated
		 *
		 * @author Julius
		 * @date 22.08.2020
		 */
		[[maybe_unused]] const state_type& state() const {
			return _state;
		}

	public:
		[[nodiscard]] nmpt<const std::atomic_flag> signal() const {
			return _state->signal();
		}

	private:
		state_type _state;
	};

	template <typename Type_>
	using Future = ::hg::concurrent::future<Type_>;
}
