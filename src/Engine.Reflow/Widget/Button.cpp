#include "Button.hpp"

#include <format>
#include "../Layout/Style.hpp"
#include "../Style/PanelStyle.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Button::Button() :
    BoxPanel(),
    _attr(
        Attributes {
            {
                .minWidth = { this, { ReflowUnitType::eAuto, 0.F } },
                .width = { this, { ReflowUnitType::eAuto, 0.F } },
                .maxWidth = { this, { ReflowUnitType::eAuto, 0.F } },
                .minHeight = { this, { ReflowUnitType::eAuto, 0.F } },
                .height = { this, { ReflowUnitType::eAuto, 0.F } },
                .maxHeight = { this, { ReflowUnitType::eAuto, 0.F } },
                .padding = { this, Padding { 0.F } },
                .justify = { this, ReflowSpacing::eStart },
                .align = { this, ReflowAlignment::eStart },
            },
            { this, ButtonStyle {} }
        }
    ) {}

Button::~Button() = default;

string Button::getTag() const noexcept {
    return std::format(R"(Button <{:#x}>)", reinterpret_cast<u64>(this));
}

ref<Button::Attributes> Button::attributes() noexcept {
    return _attr;
}

void Button::render(const ptr<ReflowCommandBuffer> cmd_) {

    PanelStyle tmp {};
    if (_state.isHover()) {
        tmp.backgroundColor = attributes().style.getValue().hoverColor;
    } else {
        tmp.backgroundColor = attributes().style.getValue().defaultColor;
    }

    Panel::renderPanel(cmd_, tmp);
    Panel::render(cmd_);
}

decltype(Button::_emitter)::handle_type Button::addOnClick(mref<decltype(_emitter)::function_type> fnc_) {
    return _emitter.on(std::move(fnc_));
}

void Button::removeOnClick(decltype(_emitter)::handle_type handle_) {
    _emitter.remove(handle_);
}

EventResponse Button::onMouseButtonDown(cref<MouseEvent> event_) {
    _emitter.emit(event_);
    return EventResponse::eConsumed;
}
