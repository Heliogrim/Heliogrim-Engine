#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Event/Event.hpp>

namespace ember::engine::input::event {

    enum class DragDropEventType : u64 {
        eDropFileType = ("Drop::File"_typeId).data,
        eDropTextType = ("Drop::Text"_typeId).data
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
            ptr<void> data_
        ) noexcept;

        DragDropEvent(cref<this_type> other_) noexcept = default;

        DragDropEvent(mref<this_type> other_) noexcept = default;

        ~DragDropEvent() noexcept = default;

    //private:
    public:
        math::ivec2 _pointer;

        DragDropEventType _type;
        ptr<void> _data;
    };

}
