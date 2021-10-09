#pragma once
#include <shared_mutex>
#include <unordered_map>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>

#include "Event.hpp"

namespace ember {

    /** The event emitter */
    class __declspec(novtable) EventEmitter {
    public:
        /**
         * The data type for the listener handle to remove listener from emitter
         */
        using handle_type = u64;
    };

    /**
     * A stateful event emitter.
     *
     * @author Julius
     * @date 18.01.2020
     */
    template <
        IsStatefulEvent EventType_,
        class MtxType_,
        bool ExternalMtx_ = false,
        class EmitLockType_ = _STD unique_lock<_STD remove_cvref_t<MtxType_>>,
        class MutateLockType_ = _STD unique_lock<_STD remove_cvref_t<MtxType_>>>
    class StatefulEventEmitterBase :
        public EventEmitter {
    public:
        using mtx_type = _STD conditional_t<ExternalMtx_,
            ref<_STD remove_cvref_t<MtxType_>>,
            _STD remove_cvref_t<MtxType_>
        >;

    public:
        /**
         * Type of the callback function to invoke
         */
        using function_type = _STD function<void(ref<EventType_>)>;

    public:
        template <class MtxType = MtxType_, typename = _STD enable_if_t<!ExternalMtx_>>
        StatefulEventEmitterBase() :
            _lhc(0),
            _listener(),
            _mtx() {}

        template <class MtxType = MtxType_, typename = _STD enable_if_t<ExternalMtx_>>
        StatefulEventEmitterBase(ref<_STD remove_cvref_t<MtxType_>> mtx_) :
            _lhc(0),
            _listener(),
            _mtx(mtx_) {}

    public:
        /**
         * Register a listener function and returns a handle
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @param  fnc_ The function.
         *
         * @returns An u64.
         */
        u64 on(mref<_STD function<void(ref<EventType_>)>> fnc_) {

            MutateLockType_ lck { _mtx };

            const auto handle { ++_lhc };
            _listener.insert(_STD make_pair(handle, _STD forward<function_type>(fnc_)));

            return handle;
        }

        /**
         * Removes a listener by the given handle
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @param  handle_ The handle to remove.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool remove(cref<handle_type> handle_) {

            MutateLockType_ lck { _mtx };

            const auto it = _listener.find(handle_);

            if (it == _listener.end()) {
                return false;
            }

            _listener.erase(it);
            return true;
        }

        void emit(ptr<EventType_> event_) {
            emit(*event_);
        }

        void emit(ref<EventType_> event_) {

            EmitLockType_ lck { _mtx };

            for (const auto& entry : _listener) {
                (entry.second)(event_);

                if (event_.canceled()) {
                    break;
                }
            }
        }

    protected:
        /**
         * Listener handle count
         */
        u64 _lhc;

        /**
         * Listener mapping
         */
        _STD unordered_map<u64, function_type> _listener;

    private:
        mtx_type _mtx;
    };

    /**
     * The stateless event emitter base.
     *
     * @author Julius
     * @date 19.01.2020
     */
    template <
        IsStatelessEvent EventType_,
        class MtxType_,
        bool ExternalMtx_ = false,
        class EmitLockType_ = _STD unique_lock<_STD remove_cvref_t<MtxType_>>,
        class MutateLockType_ = _STD unique_lock<_STD remove_cvref_t<MtxType_>>>
    class StatelessEventEmitterBase :
        public EventEmitter {
    public:
        using mtx_type = _STD conditional_t<ExternalMtx_,
            ref<_STD remove_cvref_t<MtxType_>>,
            _STD remove_cvref_t<MtxType_>
        >;

    public:
        /**
         * Type of the callback function to invoke
         */
        using function_type = _STD function<void(cref<EventType_>)>;

    public:
        template <typename MtxType = MtxType_, typename = _STD enable_if_t<!ExternalMtx_>>
        StatelessEventEmitterBase() :
            _lhc(0),
            _listener(),
            _mtx() {}

        template <typename MtxType = MtxType_, typename = _STD enable_if_t<ExternalMtx_>>
        StatelessEventEmitterBase(ref<_STD remove_cvref_t<MtxType>> mtx_) :
            _lhc(0),
            _listener(),
            _mtx(mtx_) {}

    public:
        /**
         * Register a listener function and returns a handle
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @param  fnc_ The function.
         *
         * @returns An u64.
         */
        u64 on(mref<_STD function<void(cref<EventType_>)>> fnc_) {

            MutateLockType_ lck { _mtx };

            handle_type handle { ++_lhc };
            _listener.insert(
                _STD make_pair<handle_type, function_type>(_STD move(handle), _STD forward<function_type>(fnc_)));

            return handle;
        }

        /**
         * Removes a listener by the given handle
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @param  handle_ The handle to remove.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool remove(cref<handle_type> handle_) {

            MutateLockType_ lck { _mtx };

            const auto it = _listener.find(handle_);

            if (it == _listener.end()) {
                return false;
            }

            _listener.erase(it);
            return true;
        }

        void emit(ptr<EventType_> event_) {
            emit(*event_);
        }

        void emit(cref<EventType_> event_) {

            EmitLockType_ lck { _mtx };

            for (const auto& entry : _listener) {
                (entry.second)(event_);
            }
        }

    protected:
        /**
         * Listener handle count
         */
        u64 _lhc;

        /**
         * Listener mapping
         */
        _STD unordered_map<u64, function_type> _listener;

    private:
        mtx_type _mtx;
    };

    /** Defines an alias representing the stateful event emitter */
    template <IsStatefulEvent EventType_>
    using StatefulEventEmitter = StatefulEventEmitterBase<EventType_, _STD recursive_mutex>;

    /** Defines an alias representing the stateless event emitter */
    template <IsStatelessEvent EventType_>
    using StatelessEventEmitter = StatelessEventEmitterBase<EventType_, _STD recursive_mutex>;
}
