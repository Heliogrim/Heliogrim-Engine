#include "DragDropReceiver.hpp"

using namespace hg::engine::input;
using namespace hg;

void DragDropReceiver::setOnDrag(mref<decltype(_onDrag)> onDrag_) {
    _onDrag = std::move(onDrag_);
}

void DragDropReceiver::setOnDrop(mref<decltype(_onDrag)> onDrop_) {
    _onDrop = std::move(onDrop_);
}

void DragDropReceiver::setOnMove(mref<decltype(_onDrag)> onMove_) {
    _onMove = std::move(onMove_);
}
