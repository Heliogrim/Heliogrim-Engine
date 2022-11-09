#include "Button.hpp"

#include "../Style/BoundStyleSheet.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Button::Button(mref<sptr<BoundStyleSheet>> style_) :
    HBox(_STD move(style_)) {}

Button::~Button() = default;

void Button::render(const ptr<ReflowCommandBuffer> cmd_) {

    //const engine::color cl { (_state.hover || _state.focus) ? _computedStyle.raisedColor : _computedStyle.color };

    const math::vec2 off {
        screenOffset() + math::vec2 { _computedStyle.margin.attr.x, _computedStyle.margin.attr.y }
    };
    const math::vec2 size {
        innerSize() + math::vec2 {
            _computedStyle.padding.attr.x + _computedStyle.padding.attr.z,
            _computedStyle.padding.attr.y + _computedStyle.padding.attr.w
        }
    };

    if (_computedStyle.borderRadius.attr.zero()) {

        const math::vec2 c0 { off.x, off.y };
        const math::vec2 c1 { off.x + size.x, off.y };
        const math::vec2 c2 { off.x + size.x, off.y + size.y };
        const math::vec2 c3 { off.x, off.y + size.y };

        cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

    } else {

        /**
         *
         */
        math::vec2 c0 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c1 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };
        math::vec2 c2 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };
        math::vec2 c3 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };

        cmd_->drawQuad(c0, c1, c2, c3, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c00 { off.x + _computedStyle.borderRadius.attr.x, off.y };
        math::vec2 c01 { (off.x + size.x) - _computedStyle.borderRadius.attr.y, off.y };
        math::vec2 c02 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };
        math::vec2 c03 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };

        cmd_->drawQuad(c00, c01, c02, c03, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c10 { off.x + size.x, off.y + _computedStyle.borderRadius.attr.y };
        math::vec2 c11 { off.x + size.x, (off.y + size.y) - _computedStyle.borderRadius.attr.z };
        math::vec2 c12 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };
        math::vec2 c13 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.y,
            off.y + _computedStyle.borderRadius.attr.y
        };

        cmd_->drawQuad(c10, c11, c12, c13, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c20 { (off.x + size.x) - _computedStyle.borderRadius.attr.z, off.y + size.y };
        math::vec2 c21 { off.x + _computedStyle.borderRadius.attr.w, off.y + size.y };
        math::vec2 c22 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };
        math::vec2 c23 {
            (off.x + size.x) - _computedStyle.borderRadius.attr.z,
            (off.y + size.y) - _computedStyle.borderRadius.attr.z
        };

        cmd_->drawQuad(c20, c21, c22, c23, _computedStyle.color.attr);

        /**
         *
         */
        math::vec2 c30 { off.x, (off.y + size.y) - _computedStyle.borderRadius.attr.w };
        math::vec2 c31 { off.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c32 { off.x + _computedStyle.borderRadius.attr.x, off.y + _computedStyle.borderRadius.attr.x };
        math::vec2 c33 {
            off.x + _computedStyle.borderRadius.attr.w,
            (off.y + size.y) - _computedStyle.borderRadius.attr.w
        };

        cmd_->drawQuad(c30, c31, c32, c33, _computedStyle.color.attr);

        /**
         *
         */
        cmd_->drawArc(math::vec2 {
                off.x + _computedStyle.borderRadius.attr.x,
                off.y + _computedStyle.borderRadius.attr.x
            },
            _computedStyle.borderRadius.attr.x, glm::radians(180.F), glm::radians(270.F), _computedStyle.color.attr);
        cmd_->drawArc(math::vec2 {
                off.x + size.x - _computedStyle.borderRadius.attr.y,
                off.y + _computedStyle.borderRadius.attr.y
            },
            _computedStyle.borderRadius.attr.y, glm::radians(270.F), glm::radians(360.F), _computedStyle.color.attr);
        cmd_->drawArc(math::vec2 {
                off.x + size.x - _computedStyle.borderRadius.attr.z,
                off.y + size.y - _computedStyle.borderRadius.attr.z
            },
            _computedStyle.borderRadius.attr.z, glm::radians(0.F), glm::radians(90.F), _computedStyle.color.attr);
        cmd_->drawArc(math::vec2 {
                off.x + _computedStyle.borderRadius.attr.w,
                off.y + size.y - _computedStyle.borderRadius.attr.w
            },
            _computedStyle.borderRadius.attr.w, glm::radians(90.F), glm::radians(180.F), _computedStyle.color.attr);
    }

    /* TODO: Relocate */
    for (const auto& child : *children()) {
        child->render(cmd_);
    }
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
