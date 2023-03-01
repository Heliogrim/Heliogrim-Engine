#pragma once
#include "Event.hpp"

namespace hg {
    class NetworkStartupEvent final :
        public StatelessEvent {
    public:
        using value_type = NetworkStartupEvent;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        inline static constexpr event_type_id typeId { "NetworkStartupEvent"_typeId };

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        explicit NetworkStartupEvent(ptr<void> module_) noexcept :
            _module(module_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        NetworkStartupEvent(cref<NetworkStartupEvent> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        NetworkStartupEvent(mref<NetworkStartupEvent> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        ~NetworkStartupEvent() noexcept = default;

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
        NetworkStartupEvent& operator=(cref<NetworkStartupEvent> other_) noexcept = default;

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
        NetworkStartupEvent& operator=(NetworkStartupEvent&& other_) noexcept = default;

    private:
        /** The module */
        ptr<void> _module;

    public:
        /**
         * Get the module pointer
         *
         * @author Julius
         * @date 11.10.2021
         */
        [[nodiscard]] ptr<void> module() const noexcept {
            return _module;
        }
    };
}
