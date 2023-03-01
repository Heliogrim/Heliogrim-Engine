#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

#include "DragDrop/DragDropObject.hpp"

namespace hg::engine::input::event {
    using DragDropEventType = ::hg::engine::input::DragDropObjectType;

    struct DragDropEventFilePayload final {
        Vector<string> paths;
    };

    struct DragDropEventTextPayload final {
        string data;
    };

    union DragDropEventPayload {
        ptr<void> _;
        ptr<DragDropEventFilePayload> files;
        ptr<DragDropEventTextPayload> text;
    };

    class DragDropEvent final :
        public StatelessEvent {
    public:
        using this_type = DragDropEvent;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

    public:
        inline static constexpr event_type_id typeId { "DragDropEvent"_typeId };

    public:
        explicit DragDropEvent(
            cref<math::ivec2> pointer_,
            cref<DragDropEventType> type_,
            mref<DragDropEventPayload> data_
        ) noexcept;

        DragDropEvent(cref<this_type> other_) noexcept;

        DragDropEvent(mref<this_type> other_) noexcept;

        ~DragDropEvent() noexcept;

        //private:
    public:
        math::ivec2 _pointer;

        DragDropEventType _type;
        DragDropEventPayload _data;
    };
}
