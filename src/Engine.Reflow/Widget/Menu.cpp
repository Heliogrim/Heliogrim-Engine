#include "Menu.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Menu::Menu() :
    Popup() {}

Menu::~Menu() = default;

string Menu::getTag() const noexcept {
    return _STD format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}

void Menu::openMenu() {
    Widget::_state |= WidgetStateFlagBits::eVisible;
}

void Menu::closeMenu() {
    Widget::_state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eVisible));
}

sptr<Widget> Menu::getContent() const noexcept {
    return _content;
}

void Menu::setContent(cref<sptr<Widget>> content_) {

    content_->setParent(shared_from_this());

    if (_content && _content->parent().get() == this) {
        _content->setParent(nullptr);
    }

    _content = content_;
}

void Menu::render(const ptr<ReflowCommandBuffer> cmd_) {
    /* Warning: Temporary solution; Menu should be on PopupLayer */
    if (!_content) {
        return;
    }
    _content->render(cmd_);
}

void Menu::flow(
    cref<FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<StyleKeyStack> styleStack_
) {
    _content->flow(ctx_, space_, limit_, styleStack_);
}

void Menu::shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) {

    if (!_content) {
        return;
    }

    _content->shift(ctx_, screenOffset());
}

const ptr<const Children> Menu::children() const {
    return nullptr;
}

math::vec2 Menu::outerSize() const noexcept {
    if (_content) {
        return _content->outerSize();
    }
    return math::vec2 { 0.F };
}

math::vec2 Menu::innerSize() const noexcept {
    if (_content) {
        return _content->innerSize();
    }
    return math::vec2 { 0.F };
}

EventResponse Menu::onFocus(cref<FocusEvent> event_) {
    return Popup::onFocus(event_);
}

EventResponse Menu::onBlur(cref<FocusEvent> event_) {

    // TODO: Check
    closeMenu();

    return Popup::onBlur(event_);
}
