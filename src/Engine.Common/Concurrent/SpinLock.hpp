#pragma once
#include <atomic>
#include <mutex>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "../__macro.hpp"

namespace hg::concurrent {
	constexpr static inline std::defer_lock_t defer_lock {};
	using SpinMutex = std::atomic_flag;

	/**/

	class SpinLock {
	public:
		explicit SpinLock(SpinMutex& mtx_) :
			_flag(mtx_),
			_owns(false) {
			lock();
		}

		explicit SpinLock(SpinMutex& mtx_, std::defer_lock_t) noexcept :
			_flag(mtx_),
			_owns(false) {}

		~SpinLock() {
			if (_owns) {
				_flag.clear();
			}
		}

		void lock() {
			if (_owns) {
				throw std::runtime_error(
					"You are not permitted to lock a already exclusively locked atomic spin lock."
				);
			}

			while (_flag.test_and_set(std::memory_order_seq_cst))
				/* spinning */;
			_owns = true;
		}

		bool try_lock() {
			if (_owns) {
				throw std::runtime_error(
					"You are not permitted to lock a already exclusively locked atomic spin lock."
				);
			}

			if (!_flag.test_and_set(std::memory_order_seq_cst)) {
				_owns = true;
				return true;
			}

			return false;
		}

		void unlock() {
			if (!_owns) {
				throw std::runtime_error(
					"You are not permitted to unlock an unowned atomic spin lock."
				);
			}

			_flag.clear(std::memory_order_seq_cst);
		}

		SpinLock operator=(const SpinLock&) = delete;

	private:
		std::atomic_flag& _flag;
		bool _owns;
	};

	class UnfairSpinLock {
	public:
		/**
		 * Default constructor
		 *
		 * @author Julius
		 * @date 25.11.2020
		 */
		UnfairSpinLock();

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 25.11.2020
		 */
		~UnfairSpinLock() noexcept;

		/**
		 * Acquires this 
		 *
		 * @author Julius
		 * @date 25.11.2020
		 */
		void acquire() noexcept;

		/**
		 * Attempts to acquire
		 *
		 * @author Julius
		 * @date 25.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool try_acquire() noexcept;

		/**
		 * Releases this
		 *
		 * @author Julius
		 * @date 25.11.2020
		 */
		void release();

		/**
		 * Attempts to release
		 *
		 * @author Julius
		 * @date 25.11.2020
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool try_release() noexcept;

		/**
		 * Copy Assignment operator
		 *
		 * @author Julius
		 * @date 25.11.2020
		 *
		 * @returns A shallow copy of this.
		 */
		UnfairSpinLock& operator=(const UnfairSpinLock&) = delete;

		/**
		 * Move assignment operator
		 *
		 * @author Julius
		 * @date 25.11.2020
		 *
		 * @returns A shallow copy of this.
		 */
		UnfairSpinLock& operator=(UnfairSpinLock&&) = delete;

	public:
		/**
		 * Alias for acquire to maintain std compatibility
		 */
		void lock();

		/**
		 * Alias for release to maintain std compatibility
		 */
		void unlock();

	private:
		/** The gate */
		std::atomic<hg::engine::scheduler::thread::thread_id> _gate;

		/**
		 * Spins this 
		 *
		 * @author Julius
		 * @date 25.11.2020
		 */
		void spin() noexcept;
	};
}
