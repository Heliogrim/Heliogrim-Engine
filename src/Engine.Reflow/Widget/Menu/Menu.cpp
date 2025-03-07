#include "Menu.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Move.hpp>

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

	_anchor->show(not _content ? NullWidget::instance() : _content, computeContentPosition());
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

void Menu::setTrigger(mref<SharedPtr<Widget>> trigger_) noexcept {

	auto prev = _trigger.getChild();
	if (prev && prev != NullWidget::instance()) {
		prev->dropOnClick(_triggerHandle);
		_trigger.getChild()->setParent(nullptr);
	}

	/**/

	trigger_->setParent(shared_from_this());
	_triggerHandle = trigger_->onClick(
		[this](const auto&) {
			this->openMenu();
			return EventResponse::eConsumed;
		}
	);

	_trigger.setChild(::hg::move(trigger_));
}

void Menu::setContent(mref<sptr<Widget>> content_) noexcept {
	// TODO: Update anchor if visible
	_content = ::hg::move(content_);
}

void Menu::setContentPosition(MenuContentPosition contentPosition_) noexcept {
	_contentPosition = contentPosition_;
}

void Menu::render(const ptr<ReflowCommandBuffer> cmd_) {
	_trigger.getChild()->render(cmd_);
}

PrefetchSizing Menu::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	return {
		_trigger.getChild()->getLayoutState().prefetchMinSize,
		_trigger.getChild()->getLayoutState().prefetchSize
	};
}

PassPrefetchSizing Menu::passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const {
	return _trigger.getChild()->passPrefetchSizing(axis_, passState_);
}

void Menu::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	_trigger.getChild()->getLayoutState().computeSize = passState_.computeSize;
}

void Menu::applyLayout(ref<ReflowState> state_) {
	const auto childState = state_.getStateOf(_trigger.getChild());
	childState->layoutOffset = getLayoutState().layoutOffset;
	childState->layoutSize = getLayoutState().layoutSize;
}

math::fvec2 Menu::getGrowFactor() const noexcept {
	return {};
}

math::fvec2 Menu::getShrinkFactor() const noexcept {
	return {};
}

math::vec2 Menu::computeContentPosition() {
	math::vec2 result = _layoutState.layoutOffset + math::vec2 { 0.F, _layoutState.layoutSize.y };
	// TODO: Consume menu content position attribute to relocate.
	return result;
}

EventResponse Menu::invokeOnFocus(cref<FocusEvent> event_) {
	return Widget::invokeOnFocus(event_);
}

EventResponse Menu::invokeOnBlur(cref<FocusEvent> event_) {
	closeMenu();
	return Widget::invokeOnBlur(event_);
}
