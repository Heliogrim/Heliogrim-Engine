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

DragDropEvent::DragDropEvent(cref<this_type> other_) noexcept :
    StatelessEvent(),
    _pointer(other_._pointer),
    _type(other_._type) {

    if (_type == DragDropObjectType::eFileType && other_._data.files) {
        _data.files = new DragDropEventFilePayload {
            Vector<string> { other_._data.files->paths }
        };

    } else if (_type == DragDropObjectType::eTextType && other_._data.text) {
        _data.text = new DragDropEventTextPayload {
            string { other_._data.text->data }
        };
    }
}

DragDropEvent::DragDropEvent(mref<this_type> other_) noexcept :
    StatelessEvent(),
    _pointer(other_._pointer),
    _type(_STD exchange(other_._type, DragDropEventType::eNone)) {

    if (_type == DragDropObjectType::eFileType && other_._data.files) {
        _data.files = _STD exchange(other_._data.files, nullptr);

    } else if (_type == DragDropObjectType::eTextType && other_._data.text) {
        _data.text = _STD exchange(other_._data.text, nullptr);
    }
}

DragDropEvent::~DragDropEvent() noexcept {

    if (_type == DragDropEventType::eFileType && _data.files) {
        delete _data.files;
        _data.files = nullptr;

    } else if (_type == DragDropEventType::eTextType && _data.text) {
        delete _data.text;
        _data.text = nullptr;
    }
}
