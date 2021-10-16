#pragma once
#include "Event.hpp"

namespace ember {

    class ShutdownEvent final :
        public StatelessEvent {
    public:
        using value_type = ShutdownEvent;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        inline static constexpr event_type_id type_id { "ShutdownEvent"_typeId };

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        explicit ShutdownEvent() noexcept = default;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        ShutdownEvent(cref<ShutdownEvent> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        ShutdownEvent(mref<ShutdownEvent> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        ~ShutdownEvent() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<ShutdownEvent> operator=(cref<ShutdownEvent> other_) noexcept = default;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<ShutdownEvent> operator=(ShutdownEvent&& other_) noexcept = default;
    };

}
