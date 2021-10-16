#pragma once

#include "../stdafx.h"

namespace ember::concurrent {

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
                _returned(false),
                _value(
                    _STD is_nothrow_default_constructible_v<Ty> ? new Ty() : _STD move(allocate())
                ) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 20.08.2020
             */
            ~future_state() {
                _mtx.lock();
                if constexpr (_STD is_nothrow_default_constructible_v<Ty>) {
                    delete _value;
                } else {
                    deallocate(_value);
                }
                _mtx.unlock();
            }

            /**
             * Set the returned state and notify all waiters
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             */
            FORCE_INLINE void complete() const {
                bool ex = false;
                if (!_returned.compare_exchange_strong(ex, true, _STD memory_order_relaxed))
                    throw _STD runtime_error("Try to complete a already returned future state.");
                _cv.notify_all();
            }

            /**
             * Set the internal value
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             *
             * @param  val_ The value.
             */
            template <typename = _STD enable_if_t<_STD is_nothrow_default_constructible_v<Ty>>>
            FORCE_INLINE void set(Ty&& val_) {
                bool ex = false;
                if (!_returned.compare_exchange_strong(ex, true, _STD memory_order_relaxed))
                    throw _STD runtime_error("Try to assign value to already assigned future state.");

                (*_value) = _STD move(val_);
                _cv.notify_all();
            }

            /**
             * Returns a reference to the allocated memory
             *
             * @returns A reference to a Ty*.
             */
            FORCE_INLINE Ty*& mem() const noexcept {
                return _value;
            }

            /**
             * Flag whether the state represents a returned value
             *
             * @returns True if it succeeds, false if it fails.
             */
            FORCE_INLINE bool returned() const {
                return _returned;
            }

            /**
             * Returns the internal stored value and invalidates it
             *
             * @returns A reference to a Ty&amp;
             */
            FORCE_INLINE Ty&& value() const {
                return _STD forward<Ty&&>(*_value);
            }

            /** Waits this  */
            FORCE_INLINE void wait() const {
                if (_returned)
                    return;
                _STD unique_lock<_STD mutex> lck(_mtx);
                _cv.wait(lck);
            }

        private:
            mutable _STD condition_variable _cv;
            mutable _STD mutex _mtx;
            mutable _STD atomic_bool _returned;
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
            future_state() :
                _returned(false) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 20.08.2020
             */
            ~future_state() { }

            /**
             * Set the returned state and notify all waiters
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             */
            FORCE_INLINE void complete() const {
                bool ex = false;
                if (!_returned.compare_exchange_strong(ex, true, _STD memory_order_relaxed))
                    throw _STD runtime_error("Try to complete a already returned future state.");
                _cv.notify_all();
            }

            /**
             * Set the internal value
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             */
            FORCE_INLINE void set() {
                bool ex = false;
                if (!_returned.compare_exchange_strong(ex, true, _STD memory_order_relaxed))
                    throw _STD runtime_error("Try to assign value to already assigned future state.");
                _cv.notify_all();
            }

            /**
             * Flag whether the state represents a returned value
             *
             * @returns True if it succeeds, false if it fails.
             */
            FORCE_INLINE bool returned() const {
                return _returned;
            }

            /** Waits this  */
            FORCE_INLINE void wait() const {
                if (_returned)
                    return;
                _STD unique_lock<_STD mutex> lck(_mtx);
                _cv.wait(lck);
            }

        private:
            mutable _STD condition_variable _cv;
            mutable _STD mutex _mtx;
            mutable _STD atomic_bool _returned;
        };
    }

    /**
     * Future
     */
    template <typename Ty>
    class future {
    public:
        using state_type = _STD shared_ptr<future_state<Ty>>;

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
            return _STD move(_state->value());
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
            return _STD move(_state->value());
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

    private:
        state_type _state;
    };

    template <>
    class future<void> {
    public:
        using state_type = _STD shared_ptr<future_state<void>>;

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
         */
        [[maybe_unused]] void retrieve() const noexcept { }

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

    private:
        state_type _state;
    };
}
