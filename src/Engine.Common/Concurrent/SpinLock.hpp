#pragma once
#include <atomic>
#include <mutex>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "../__macro.hpp"

namespace hg::concurrent {
    /**
     * \brief 
     */
    _INLINE_VAR constexpr std::defer_lock_t defer_lock {};

    /**
     * \brief 
     */
    using SpinMutex = _STD atomic_flag;

    /**
     * \brief 
     */
    class SpinLock {
    public:
        /**
         * \brief
         * \param mtx_ 
         */
        explicit SpinLock(SpinMutex& mtx_) :
            _flag(mtx_),
            _owns(false) {
            lock();
        }

        /**
         * \brief 
         * \param mtx_ 
         */
        explicit SpinLock(SpinMutex& mtx_, _STD defer_lock_t) noexcept :
            _flag(mtx_),
            _owns(false) { }

        /**
         * \brief 
         */
        ~SpinLock() {
            if (_owns) {
                _flag.clear();
            }
        }

        /**
         * \brief 
         */
        void lock() {
            if (_owns) {
                _Throw_system_error(_STD errc::operation_not_permitted);
            }

            while (_flag.test_and_set(_STD memory_order_seq_cst))
                /* spinning */;
            _owns = true;
        }

        /**
         * \brief 
         * \return 
         */
        bool try_lock() {
            if (_owns) {
                std::_Throw_system_error(_STD errc::operation_not_permitted);
            }

            if (!_flag.test_and_set(_STD memory_order_seq_cst)) {
                _owns = true;
                return true;
            }

            return false;
        }

        /**
         * \brief 
         */
        void unlock() {
            if (!_owns) {
                _Throw_system_error(_STD errc::operation_not_permitted);
            }

            _flag.clear(_STD memory_order_seq_cst);
        }

        /**
         * \brief 
         * \return 
         */
        SpinLock operator=(const SpinLock&) = delete;

    private:
        /**
         * \brief 
         */
        _STD atomic_flag& _flag;

        /**
         * \brief 
         */
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

    private:
        /** The gate */
        _STD atomic<hg::engine::scheduler::thread::thread_id> _gate;

        /**
         * Spins this 
         *
         * @author Julius
         * @date 25.11.2020
         */
        FORCE_INLINE void spin() noexcept;
    };
}
