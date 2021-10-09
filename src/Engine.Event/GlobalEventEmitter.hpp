#pragma once

#include <Engine.Common/Collection/BytellHashMap.hpp>

#include "EventEmitter.hpp"

namespace ember {

    class GlobalEventEmitter {
    public:
        using value_type = GlobalEventEmitter;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using emitter_mtx_value_type = _STD recursive_mutex;
        using emitter_mtx_type = ref<emitter_mtx_value_type>;

        template <IsStatefulEvent EventType_>
        using stateful_emitter_type = StatefulEventEmitterBase<EventType_, emitter_mtx_type, true>;

        template <IsStatelessEvent EventType_>
        using stateless_emitter_type = StatelessEventEmitterBase<EventType_, emitter_mtx_type, true>;

    private:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 07.10.2021
         */
        GlobalEventEmitter() noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 07.10.2021
         */
        ~GlobalEventEmitter() noexcept;

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
         * The singleton instance
         */
        static ptr<value_type> _instance;

    public:
        /**
         * Gets the singleton instance
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @returns A const ptr&lt;value_type&gt;
         */
        [[nodiscard]] static const ptr<value_type> get() noexcept;

        /**
         * Makes a singleton instance if not exist and returns a pointer
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @returns A const ptr&lt;value_type&gt;
         */
        static const ptr<value_type> make() noexcept;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 07.10.2021
         */
        static void destroy() noexcept;

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
        ska::bytell_hash_map<event_type_id, ptr<EventEmitter>> _emitter;

    public:
        template <IsStatefulEvent EventType_>
        void emit(ref<EventType_> event_) {

            const auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->emit(event_);
        }

        template <IsStatelessEvent EventType_>
        void emit(cref<EventType_> event_) {

            const auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                return;
            }

            static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->emit(event_);
        }

        template <IsEvent EventType_>
        void emit(ptr<EventType_> event_) {

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

            auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                auto result = _emitter.insert(_STD make_pair(EventType_::type_id, makeEmitter<EventType_>()));
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

            auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                auto result = _emitter.insert(_STD make_pair(EventType_::type_id, makeEmitter<EventType_>()));
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

            auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                return false;
            }

            return static_cast<ptr<stateful_emitter_type<EventType_>>>(it->second)->remove(handle_);
        }

        template <IsStatelessEvent EventType_>
        bool remove(cref<typename stateless_emitter_type<EventType_>::handle_type> handle_) {

            auto it = _emitter.find(EventType_::type_id);
            if (it == _emitter.end()) {
                return false;
            }

            return static_cast<ptr<stateless_emitter_type<EventType_>>>(it->second)->remove(handle_);
        }
    };
}
