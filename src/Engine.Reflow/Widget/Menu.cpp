#include "Menu.hpp"

#include <format>

using namespace hg::engine::reflow;
using namespace hg;

Menu::Menu() :
	Popup() {}

Menu::~Menu() = default;

string Menu::getTag() const noexcept {
	return std::format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}

void Menu::openMenu() {
	Widget::_state |= WidgetStateFlagBits::eVisible;
}

void Menu::closeMenu() {
	Widget::_state.unwrap &= (~static_cast<WidgetState::value_type>(WidgetStateFlagBits::eVisible));
}

const ptr<const SingleChildren> Menu::children() const {
	return &_children;
}

sptr<Widget> Menu::getContent() const noexcept {
	return _children.getChild();
}

void Menu::setContent(cref<sptr<Widget>> content_) {

	content_->setParent(shared_from_this());

	if (_children.getChild() && _children.getChild()->parent().get() == this) {
		_children.getChild()->setParent(nullptr);
	}

	_children.setChild(content_);
}

void Menu::render(const ptr<ReflowCommandBuffer> cmd_) {
	/* Warning: Temporary solution; Menu should be on PopupLayer */
	_children.getChild()->render(cmd_);
}

math::vec2 Menu::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _children.getChild()->getDesiredSize();
}

math::vec2 Menu::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return _children.getChild()->getDesiredSize();
}

void Menu::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	const auto childState = state_.getStateOf(_children.getChild());
	childState->layoutOffset = ctx_.localOffset;
	childState->layoutSize = ctx_.localSize;
}

EventResponse Menu::onFocus(cref<FocusEvent> event_) {
	return Popup::onFocus(event_);
}

EventResponse Menu::onBlur(cref<FocusEvent> event_) {

	// TODO: Check
	closeMenu();

	return Popup::onBlur(event_);
}
