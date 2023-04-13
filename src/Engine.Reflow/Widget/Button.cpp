#include "Button.hpp"

#include "../Layout/Style.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Button::Button() :
    BoxPanel() {}

Button::~Button() = default;

string Button::getTag() const noexcept {
    return _STD format(R"(Button <{:#x}>)", reinterpret_cast<u64>(this));
}

decltype(Button::_emitter)::handle_type Button::addOnClick(mref<decltype(_emitter)::function_type> fnc_) {
    return _emitter.on(_STD move(fnc_));
}

void Button::removeOnClick(decltype(_emitter)::handle_type handle_) {
    _emitter.remove(handle_);
}

EventResponse Button::onMouseButtonDown(cref<MouseEvent> event_) {
    _emitter.emit(event_);
    return EventResponse::eConsumed;
}
