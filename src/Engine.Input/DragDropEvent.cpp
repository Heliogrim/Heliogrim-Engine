#include "DragDropEvent.hpp"

using namespace ember::engine::input::event;
using namespace ember;

DragDropEvent::DragDropEvent(cref<math::ivec2> pointer_, cref<DragDropEventType> type_, ptr<void> data_) noexcept :
    StatelessEvent(),
    _pointer(pointer_),
    _type(type_),
    _data(data_) {}
