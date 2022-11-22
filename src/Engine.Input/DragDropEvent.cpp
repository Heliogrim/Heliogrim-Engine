#include "DragDropEvent.hpp"

using namespace ember::engine::input::event;
using namespace ember;

DragDropEvent::DragDropEvent(
    cref<math::ivec2> pointer_,
    cref<DragDropEventType> type_,
    mref<DragDropEventPayload> data_
) noexcept :
    StatelessEvent(),
    _pointer(pointer_),
    _type(type_),
    _data(_STD move(data_)) {}

DragDropEvent::~DragDropEvent() noexcept {

    if (_type == DragDropEventType::eFileType && _data.files) {
        delete _data.files;
        _data.files = nullptr;

    } else if (_type == DragDropEventType::eTextType && _data.text) {
        delete _data.text;
        _data.text = nullptr;
    }
}
