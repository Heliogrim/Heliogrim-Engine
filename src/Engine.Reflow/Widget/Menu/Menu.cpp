#include "Menu.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>

#include "../Button.hpp"
#include "../Layer/Anchor.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Menu::Menu() :
	Widget(),
	_triggerHandle(),
	_trigger(),
	_content() {
	/**/
	_anchor = make_sptr<Anchor>();
	//_anchor->setParent(shared_from_this());
	_anchor->state().unset(WidgetStateFlagBits::eVisible);
}

Menu::~Menu() = default;

string Menu::getTag() const noexcept {
	return std::format(R"(Menu <{:#x}>)", reinterpret_cast<u64>(this));
}

void Menu::openMenu() {
	_anchor->setParent(shared_from_this());
	_anchor->state().set(WidgetStateFlagBits::eVisible);

	_anchor->show(not _content ? NullWidget::instance() : _content);
}

void Menu::closeMenu() {
	_anchor->setParent(shared_from_this());
	_anchor->state().unset(WidgetStateFlagBits::eVisible);

	_anchor->hide();
}

const ptr<const SingleChildren> Menu::children() const {
	return &_trigger;
}

sptr<Widget> Menu::getTrigger() const noexcept {
	return _trigger.getChild();
}

sptr<Widget> Menu::getContent() const noexcept {
	return _content;
}

void Menu::setTrigger(mref<sptr<Button>> trigger_) noexcept {

	auto prev = _trigger.getChild();
	if (prev && prev != NullWidget::instance()) {

		auto prevTrigger = std::static_pointer_cast<Button, Widget>(prev);
		prevTrigger->removeOnClick(_triggerHandle);

		_trigger.getChild()->setParent(nullptr);
	}

	/**/

	trigger_->setParent(shared_from_this());
	_triggerHandle = trigger_->addOnClick(
		[this](const auto&) {
			this->openMenu();
		}
	);

	_trigger.setChild(::hg::move(trigger_));
}

void Menu::setContent(mref<sptr<Widget>> content_) noexcept {
	// TODO: Update anchor if visible
	_content = ::hg::move(content_);
}

void Menu::render(const ptr<ReflowCommandBuffer> cmd_) {
	_trigger.getChild()->render(cmd_);
}

math::vec2 Menu::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _trigger.getChild()->getDesiredSize();
}

math::vec2 Menu::computeDesiredSize(cref<ReflowPassState> passState_) const {
	return _trigger.getChild()->getDesiredSize();
}

#include <Engine.Logging/Logger.hpp>

void Menu::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	const auto childState = state_.getStateOf(_trigger.getChild());
	childState->layoutOffset = ctx_.localOffset;
	childState->layoutSize = ctx_.localSize;

	Logger::debug(
		"Menu: {}:{} / {}:{}",
		childState->layoutOffset.x,
		childState->layoutOffset.y,
		childState->layoutSize.x,
		childState->layoutSize.y
	);
}

EventResponse Menu::onFocus(cref<FocusEvent> event_) {
	//return Popup::onFocus(event_);
	return Widget::onFocus(event_);
}

EventResponse Menu::onBlur(cref<FocusEvent> event_) {
	//closeMenu();
	//return Popup::onBlur(event_);
	closeMenu();
	return Widget::onBlur(event_);
}
