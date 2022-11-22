#include "DragDropReceiver.hpp"

using namespace ember::engine::input;
using namespace ember;

void DragDropReceiver::setOnDrag(mref<decltype(_onDrag)> onDrag_) {
    _onDrag = _STD move(onDrag_);
}

void DragDropReceiver::setOnDrop(mref<decltype(_onDrag)> onDrop_) {
    _onDrop = _STD move(onDrop_);
}

void DragDropReceiver::setOnMove(mref<decltype(_onDrag)> onMove_) {
    _onMove = _STD move(onMove_);
}
