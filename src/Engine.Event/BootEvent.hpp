#pragma once
#include "Event.hpp"

namespace ember {
    class BootEvent final :
        public StatelessEvent {
    public:
        using value_type = BootEvent;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        inline static constexpr event_type_id typeId { "BootEvent"_typeId };

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  session_ The session.
         */
        explicit BootEvent(cref<ptr<void>> session_) noexcept :
            _session(session_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        BootEvent(cref<BootEvent> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        BootEvent(mref<BootEvent> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        ~BootEvent() noexcept = default;

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
        BootEvent& operator=(cref<BootEvent> other_) = default;

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
        BootEvent& operator=(BootEvent&& other_) noexcept = default;

    private:
        /** The session */
        ptr<void> _session;

    public:
        [[nodiscard]] ptr<void> session() const noexcept {
            return _session;
        }
    };
}
