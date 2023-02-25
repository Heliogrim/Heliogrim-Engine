#pragma once

#include <functional>

#include "Future.hpp"
#include "../Make.hpp"

namespace ember::concurrent {
    namespace {
        /**
         * The internal state of a promise.
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @tparam Ry Type of the return value alias expected result.
         * @tparam Args Packed Type of the arguments to invoke the function.
         */
        template <typename Ry, typename... Args>
        class promise_state {
        public:
            using future_state_type = typename future<Ry>::state_type;
            using shared_type = _STD shared_ptr<promise_state<Ry, Args...>>;

            using fnc_type = _STD function<Ry(Args...)>;
            using next_fnc_type = _STD function<void(Ry&&)>;

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param [in,out] fnc_ The function.
             */
            promise_state(fnc_type&& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param  fnc_ The function.
             */
            promise_state(const fnc_type& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 04.09.2020
             */
            ~promise_state() = default;

            /**
             * Returns a reference to the shared future if no chain created
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @returns A reference to a future_state_type.
             */
            future_state_type& future() {
                if (_next != nullptr)
                    throw _STD logic_error(
                        "You can not capture the returned value of promise intersecting the promise chain."
                    );
                if (!_future)
                    _future = make_sptr<typename future_state_type::element_type>();
                return _future;
            }

            /**
             * Steps through the current state
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @param args_ The invocation arguments for the stored function.
             */
            void step(Args... args_) {
                if (_future != nullptr) {
                    // Store result to internal future
                    _future->set(
                        // Forward invocation result to storage
                        _STD forward<Ry>(
                            // Invoke function with forwarded parameter list
                            _fnc(_STD forward<Args>(args_)...)
                        )
                    );
                    // Reset function to unlink context
                    _fnc = nullptr;
                    return;
                }

                if (_next != nullptr) {
                    // Invoke next function with forwarded result
                    _next(
                        // Forward invocation result to next function
                        _STD forward<Ry>(
                            // Invoke function with forwarded parameter list
                            _fnc(_STD forward<Args>(args_)...)
                        )
                    );
                    // Reset function to unlink context
                    _fnc = nullptr;
                    // Reset next function to unlink context -> will release anonymous lambda for next future state
                    _next = nullptr;
                    return;
                }

                // TODO: create future state and store
                _fnc(_STD forward<Args>(args_)...);
                // Reset function to unlink context
                _fnc = nullptr;
            }

            /**
             * Stacks the given next
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @param  next_ The next.
             */
            void stack(const _STD function<void(Ry&&)>&& next_) {
                _next = _STD move(next_);
            }

        protected:
            mutable fnc_type _fnc;
            mutable next_fnc_type _next;

            mutable future_state_type _future;
        };

        template <typename... Args>
        class promise_state<void, Args...> {
        public:
            using future_state_type = typename future<void>::state_type;
            using shared_type = _STD shared_ptr<promise_state<void, Args...>>;

            using fnc_type = _STD function<void(Args...)>;
            using next_fnc_type = _STD function<void()>;

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param [in,out] fnc_ The function.
             */
            promise_state(fnc_type&& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param  fnc_ The function.
             */
            promise_state(const fnc_type& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 04.09.2020
             */
            ~promise_state() = default;

            /**
             * Returns a reference to the shared future if no chain created
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @returns A reference to a future_state_type.
             */
            future_state_type& future() {
                if (!_future)
                    _future = make_sptr<typename future_state_type::element_type>();
                return _future;
            }

            /**
             * Steps through the current state
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @param args_ The invocation arguments for the stored function.
             */
            void step(Args... args_) {
                if (_future != nullptr) {
                    // Store state to internal future
                    _future->set(
                        // Invoke function with forwarded parameter list
                        _fnc(_STD forward(args_)...)
                    );
                    // Reset function to unlink context
                    _fnc = nullptr;
                    return;
                }

                if (_next != nullptr) {
                    // Invoke next function
                    _next(
                        // Forward invocation result to next function
                        _STD forward(
                            // Invoke function with forwarded parameter list
                            _fnc(_STD forward(args_)...)
                        )
                    );
                    // Reset function to unlink context
                    _fnc = nullptr;
                    // Reset next function to unlink context -> will release anonymous lambda for next future state
                    _next = nullptr;
                    return;
                }

                // TODO: create future state and store
                _fnc(_STD forward(args_)...);
                // Reset function to unlink context
                _fnc = nullptr;
            }

            /**
             * Stacks the given next
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @param  next_ The next.
             */
            void stack(const _STD function<void()>&& next_) {
                _next = _STD move(next_);
            }

        protected:
            mutable fnc_type _fnc;
            mutable next_fnc_type _next;

            mutable future_state_type _future;
        };

        template <>
        class promise_state<void, void> {
        public:
            using future_state_type = typename future<void>::state_type;
            using shared_type = _STD shared_ptr<promise_state<void, void>>;

            using fnc_type = _STD function<void(void)>;
            using next_fnc_type = _STD function<void()>;

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param [in,out] fnc_ The function.
             */
            promise_state(fnc_type&& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 03.09.2020
             *
             * @param  fnc_ The function.
             */
            promise_state(const fnc_type& fnc_) :
                _fnc(_STD move(fnc_)),
                _next(nullptr),
                _future(nullptr) {}

            /**
             * Destructor
             *
             * @author Julius
             * @date 04.09.2020
             */
            ~promise_state() = default;

            /**
             * Returns a reference to the shared future if no chain created
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @returns A reference to a future_state_type.
             */
            future_state_type& future() {
                if (!_future)
                    _future = make_sptr<typename future_state_type::element_type>();
                return _future;
            }

            /**
             * Steps through the current state
             *
             * @author Julius
             * @date 22.08.2020
             */
            void step() {
                if (_future != nullptr) {
                    // Invoke function with forwarded parameter list
                    _fnc();
                    // Store state to internal future
                    _future->set();
                    // Reset function to unlink context
                    _fnc = nullptr;
                    return;
                }

                if (_next != nullptr) {
                    // Invoke function with forwarded parameter list
                    _fnc();
                    // Invoke next function
                    _next();
                    // Reset function to unlink context
                    _fnc = nullptr;
                    // Reset next function to unlink context -> will release anonymous lambda for next future state
                    _next = nullptr;
                    return;
                }

                // TODO: create future state and store
                _fnc();
                // Reset function to unlink context
                _fnc = nullptr;
            }

            /**
             * Stacks the given next
             *
             * @author Julius
             * @date 22.08.2020
             *
             * @param  next_ The next.
             */
            void stack(const _STD function<void()>&& next_) {
                _next = _STD move(next_);
            }

        protected:
            mutable fnc_type _fnc;
            mutable next_fnc_type _next;

            mutable future_state_type _future;
        };
    }

    template <typename Ret>
    class promise;

    template <typename Ret, typename... Args>
    class nxpromise {
        template <typename Ty>
        friend class promise;

        template <typename NxRet, typename... NxArgs>
        friend class nxpromise;

    public:
        using value_type = nxpromise<Ret, Args...>;
        using reference_type = nxpromise<Ret, Args...>&;
        using const_reference_type = const nxpromise<Ret, Args...>&;

        using fnc_type = _STD function<Ret(Args...)>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.09.2020
         */
        ~nxpromise() = default;

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param [in,out] fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, Ret> then(_STD function<NxRet(Ret)>&& fnc_) {
            nxpromise<NxRet, Ret> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state](Ret&& args_) {
                    nxs->step(_STD forward<Ret>(args_));
                }
            );

            return _STD forward<nxpromise<NxRet, Ret>>(next);
        }

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, Ret> then(const _STD function<NxRet(Ret)>& fnc_) {
            nxpromise<NxRet, Ret> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state](Ret&& args_) {
                    nxs->step(_STD forward(args_));
                }
            );

            return _STD forward(next);
        }

        /**
         * Stacks the given function as last invocation of the chain
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param  fnc_ The function.
         */
        void finally(const _STD function<void(Ret)>&& fnc_) {
            _state->stack(_STD move(fnc_));
        }

        /**
         * Returns a future object from promise chain, if possible
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @throws Error occurs if future should be return intersecting the chain
         *
         * @returns A future instance of type future<Ret> provided by promise chain;
         */
        future<Ret> get() {
            typename future<Ret>::state_type& fs = _state->future();
            return _STD move(future<Ret>(fs));
        }

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param [in,out] fnc_ The function.
         */
        nxpromise(fnc_type&& fnc_) :
            _state(make_sptr<promise_state<Ret, Args...>>(_STD move(fnc_))) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 03.09.2020
         *
         * @param  fnc_ The function.
         */
        nxpromise(const fnc_type& fnc_) :
            _state(make_sptr<promise_state<Ret, Args...>>(_STD move(fnc_))) {}

    private:
        mutable typename promise_state<Ret, Args...>::shared_type _state;
    };

    /**
     * Promise
     */
    template <typename Ret>
    class promise {
    public:
        using value_type = promise<Ret>;
        using reference_type = promise<Ret>&;
        using const_reference_type = const promise<Ret>&;

        using fnc_type = _STD function<Ret()>;

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param [in,out] fnc_ The function.
         */
        promise(fnc_type&& fnc_) :
            _state(make_sptr<promise_state<Ret>>(_STD move(fnc_))) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param  fnc_ The function.
         */
        promise(const fnc_type& fnc_) :
            _state(make_sptr<promise_state<Ret>>(_STD move(fnc_))) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.09.2020
         */
        ~promise() = default;

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param [in,out] fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, Ret> then(_STD function<NxRet(Ret)>&& fnc_) {
            nxpromise<NxRet, Ret> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state](Ret&& args_) {
                    return nxs->step(_STD forward<Ret>(args_));
                }
            );

            return _STD forward<nxpromise<NxRet, Ret>>(next);
        }

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, Ret> then(const _STD function<NxRet(Ret)>& fnc_) {
            nxpromise<NxRet, Ret> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state](Ret&& args_) {
                    return nxs->step(_STD forward(args_));
                }
            );

            return _STD forward(next);
        }

        /**
         * Stacks the given function as last invocation of the chain
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param fnc_ The function.
         */
        void finally(const _STD function<void(Ret)>&& fnc_) {
            _state->stack(_STD move(fnc_));
        }

        /**
         * Returns a future object from promise chain, if possible
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @throws Error occurs if future should be return intersecting the chain
         *
         * @returns A future instance of type future<Ret> provided by promise chain;
         */
        future<Ret> get() {
            typename future<Ret>::state_type& fs = _state->future();
            return _STD move(future<Ret>(fs));
        }

        /**
         * Function call operator
         *
         * @author Julius
         * @date 22.08.2020
         */
        void operator()() const {
            _state->step();
        }

    private:
        mutable typename promise_state<Ret>::shared_type _state;
    };

    template <>
    class promise<void> {
    public:
        using value_type = promise<void>;
        using reference_type = promise<void>&;
        using const_reference_type = const promise<void>&;

        using fnc_type = _STD function<void()>;

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param [in,out] fnc_ The function.
         */
        promise(fnc_type&& fnc_) :
            _state(make_sptr<promise_state<void, void>>(_STD move(fnc_))) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param  fnc_ The function.
         */
        promise(const fnc_type& fnc_) :
            _state(make_sptr<promise_state<void, void>>(_STD move(fnc_))) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.09.2020
         */
        ~promise() = default;

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param [in,out] fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, void> then(_STD function<NxRet(void)>&& fnc_) {
            nxpromise<NxRet, void> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state]() {
                    return nxs->step();
                }
            );

            return _STD forward<nxpromise<NxRet, void>>(next);
        }

        /**
         * Stacks the given function and returns the next promise in range
         *
         * @tparam NxRet Type of the returned value of the submitted function.
         * @param fnc_ The function.
         *
         * @returns A chained promise instance of type nxpromise<NxRet, Ret>
         */
        template <typename NxRet>
        [[nodiscard]] nxpromise<NxRet, void> then(const _STD function<NxRet(void)>& fnc_) {
            nxpromise<NxRet, void> next(_STD move(fnc_));

            // Anonymous Subscriber
            _state->stack(
                [nxs = next._state]() {
                    return nxs->step();
                }
            );

            return _STD forward(next);
        }

        /**
         * Stacks the given function as last invocation of the chain
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @param fnc_ The function.
         */
        void finally(const _STD function<void(void)>&& fnc_) {
            _state->stack(_STD move(fnc_));
        }

        /**
         * Returns a future object from promise chain, if possible
         *
         * @author Julius
         * @date 22.08.2020
         *
         * @throws Error occurs if future should be return intersecting the chain
         *
         * @returns A future instance of type future<Ret> provided by promise chain;
         */
        [[nodiscard]] future<void> get() {
            typename future<void>::state_type& fs = _state->future();
            return future<void>(fs);
        }

        /**
         * Function call operator
         *
         * @author Julius
         * @date 22.08.2020
         */
        void operator()() const {
            _state->step();
        }

    private:
        mutable typename promise_state<void, void>::shared_type _state;
    };
}
