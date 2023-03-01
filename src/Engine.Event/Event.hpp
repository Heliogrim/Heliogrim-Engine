#pragma once

#include <concepts>

#include "EventTypeId.hpp"

namespace hg {
    class __declspec(novtable) Event { };

    class __declspec(novtable) StatelessEvent : public Event {};

    class __declspec(novtable) StatefulEvent : public Event {
    public:
        /**
         * Destructor
         *
         *  Warning: This destructor is just to suppress warnings/errors; (novtable) will erase any class reference from vtable
         *
         * @author Julius
         * @date 07.10.2021
         */
        virtual ~StatefulEvent() = default;

    public:
        /**
         * Check if this event was canceled
         *
         * @author Julius
         * @date 07.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] virtual bool canceled() const noexcept = 0;

        /**
         * Cancel this event
         *
         * @author Julius
         * @date 07.10.2021
         */
        virtual void cancel() noexcept = 0;
    };

    template <class EventType_>
    concept IsEvent = hg::HasStaticType<EventType_, event_type_id>;

    template <class EventType_>
    concept IsStatefulEvent = IsEvent<EventType_> && requires(const EventType_ obj) {
            { obj.canceled() } -> std::same_as<bool>;
        } && requires(EventType_ obj) {
            { obj.cancel() } -> std::same_as<void>;
        };

    template <class EventType_>
    concept IsStatelessEvent = IsEvent<EventType_> && !IsStatefulEvent<EventType_>;
}
