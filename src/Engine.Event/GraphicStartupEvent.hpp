#pragma once
#include "Event.hpp"

namespace ember {

    class GraphicStartupEvent final :
        public StatelessEvent {
    public:
        using value_type = GraphicStartupEvent;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        inline static constexpr event_type_id typeId { "GraphicStartupEvent"_typeId };

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        explicit GraphicStartupEvent(ptr<void> module_) noexcept :
            _module(module_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        GraphicStartupEvent(cref<GraphicStartupEvent> other_) noexcept = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 11.10.2021
         *
         * @param  other_ The other.
         */
        GraphicStartupEvent(mref<GraphicStartupEvent> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 11.10.2021
         */
        ~GraphicStartupEvent() noexcept = default;

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
        ref<GraphicStartupEvent> operator=(cref<GraphicStartupEvent> other_) noexcept = default;

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
        GraphicStartupEvent& operator=(GraphicStartupEvent&& other_) noexcept = default;

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
