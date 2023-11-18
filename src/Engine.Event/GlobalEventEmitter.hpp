#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include <Engine.Common/Collection/DenseMap.hpp>

#include "EventEmitter.hpp"

namespace hg {
    /*
    namespace {

        template <IsStatelessEvent EventType_>
        struct GlobalStatelessEventExecutor :
            public StatelessEventExecutor<EventType_> {
            FORCE_INLINE void operator()(cref<_STD function<void(cref<EventType_>)>> fnc_, cref<EventType_> event_) {
                hg::engine::Scheduler::get().exec(hg::engine::scheduler::task::make_task([&]() {
                    fnc_(event_);
                }));
            }
        };

    };
     */

    class GlobalEventEmitter {
    public:
        using value_type = GlobalEventEmitter;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using emitter_mtx_value_type = _STD recursive_mutex;
        using emitter_mtx_type = emitter_mtx_value_type;

        template <IsStatefulEvent EventType_>
        using stateful_emitter_type = StatefulEventEmitterBase<EventType_, emitter_mtx_type, true>;

        template <IsStatelessEvent EventType_>
        using stateless_emitter_type = StatelessEventEmitterBase<
            EventType_,
            emitter_mtx_type,
            true
            // TODO: GlobalStatelessEventExecutor<EventType_>
        >;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 07.10.2021
         */
        GlobalEventEmitter() noexcept;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 16.10.2021
         */
        GlobalEventEmitter(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 16.10.2021
         */
        GlobalEventEmitter(mref<value_type>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 07.10.2021
         */
        ~GlobalEventEmitter() noexcept;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 16.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 16.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) noexcept = delete;

    private:
        /**
         * Tidies this up
         *
         * @author Julius
         * @date 07.10.2021
         */
        void tidy() noexcept;

    private:
        /**
         * The shared mutex for the emitter
         */
        emitter_mtx_value_type _emitterMtx;

    private:
        template <IsStatelessEvent EventType_>
        [[nodiscard]] ptr<stateless_emitter_type<EventType_>> makeEmitter() {
            using emitter_type = stateless_emitter_type<EventType_>;
            return new emitter_type(_emitterMtx);
        }

        template <IsStatefulEvent EventType_>
        [[nodiscard]] ptr<stateful_emitter_type<EventType_>> makeEmitter() {
            using emitter_type = stateful_emitter_type<EventType_>;
            return new emitter_type(_emitterMtx);
        }

    private:
        /** The emitter */
        DenseMap<event_type_id, ptr<EventEmitter>> _emitter;

    public:
        template <IsStatefulEvent EventType_>
        void emit(ref<EventType_> event_) const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->emit(event_);
        }

        template <IsStatefulEvent EventType_> requires _STD is_trivially_default_constructible_v<EventType_>
        void emit() const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->emit(EventType_ {});
        }

        template <IsStatefulEvent EventType_, typename Arg0_, typename... Args_> requires _STD is_constructible_v<
            EventType_, Arg0_, Args_...>
        void emit(Arg0_&& arg0_, Args_&&... args_) const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->emit(
                EventType_ {
                    _STD forward<Arg0_>(arg0_),
                    _STD forward<Args_>(args_)...
                }
            );
        }

        template <IsStatelessEvent EventType_>
        void emit(cref<EventType_> event_) const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->emit(event_);
        }

        template <IsStatelessEvent EventType_> requires _STD is_trivially_default_constructible_v<EventType_>
        void emit() const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->emit(EventType_ {});
        }

        template <IsStatelessEvent EventType_, typename Arg0_, typename... Args_> requires _STD is_constructible_v<
            EventType_, Arg0_, Args_...>
        void emit(Arg0_&& arg0_, Args_&&... args_) const {

            const auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->emit(
                EventType_ {
                    _STD forward<Arg0_>(arg0_),
                    _STD forward<Args_>(args_)...
                }
            );
        }

        template <IsEvent EventType_>
        void emit(ptr<EventType_> event_) const {

            if constexpr (IsStatefulEvent<EventType_>) {
                emit<EventType_>(static_cast<ref<EventType_>>(*event_));
            } else {
                emit<EventType_>(static_cast<cref<EventType_>>(*event_));
            }
        }

    public:
        template <IsStatefulEvent EventType_>
        typename stateful_emitter_type<EventType_>::handle_type on(
            mref<typename stateful_emitter_type<EventType_>::function_type> fnc_
        ) {

            auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                auto result = _emitter.insert(_STD make_pair(EventType_::typeId, makeEmitter<EventType_>()));
                DEBUG_ASSERT(result.second, "Insert should be successful")

                it = result.first;
            }

            return static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->on(
                _STD forward<typename stateful_emitter_type<EventType_>::function_type>(fnc_)
            );
        }

        template <IsStatelessEvent EventType_>
        typename stateless_emitter_type<EventType_>::handle_type on(
            mref<typename stateless_emitter_type<EventType_>::function_type> fnc_
        ) {

            auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                auto result = _emitter.insert(_STD make_pair(EventType_::typeId, makeEmitter<EventType_>()));
                DEBUG_ASSERT(result.second, "Insert should be successful")

                it = result.first;
            }

            return static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->on(
                _STD forward<typename stateless_emitter_type<EventType_>::function_type>(fnc_)
            );
        }

    public:
        template <IsStatefulEvent EventType_>
        bool remove(cref<typename stateful_emitter_type<EventType_>::handle_type> handle_) {

            auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return false;
            }

            return static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->remove(handle_);
        }

        template <IsStatelessEvent EventType_>
        bool remove(cref<typename stateless_emitter_type<EventType_>::handle_type> handle_) {

            auto it = _emitter.find(EventType_::typeId);
            if (it == _emitter.end()) {
                return false;
            }

            return static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->remove(handle_);
        }
    };
}
