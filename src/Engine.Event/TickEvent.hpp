#pragma once
#include "Event.hpp"

namespace hg {
    class TickEvent final :
        public StatelessEvent {
    public:
        using value_type = TickEvent;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        inline static constexpr event_type_id typeId { "TickEvent"_typeId };

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 08.10.2021
         *
         * @param  tick_ The tick.
         */
        explicit TickEvent(const u64 tick_) noexcept :
            tick(tick_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 08.10.2021
         *
         * @param  other_ The other.
         */
        TickEvent(cref<TickEvent> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 08.10.2021
         *
         * @param  other_ The other.
         */
        TickEvent(mref<TickEvent> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 08.10.2021
         */
        ~TickEvent() noexcept = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 08.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 08.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<value_type>) = delete;

    public:
        const u64 tick;
    };
}
