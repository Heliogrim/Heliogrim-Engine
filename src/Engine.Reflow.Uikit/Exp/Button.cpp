#include "Button.hpp"

#include <format>
#include <Engine.Common/Functional/Lambda.hpp>
#include <Engine.Reflow.Theming/Theming.hpp>
#include <Engine.Reflow/ReflowClassList.hpp>

#include "Themes.hpp"
#include "../Atom/Paint.hpp"
#include "../Atom/Layout/BoxLayout.hpp"

using namespace hg::engine::reflow;
using namespace hg;

SharedPtr<uikit::Button> uikit::makeButton(
	mref<Variant<TextButtonCreateOptions, ImageButtonCreateOptions, ButtonCreateOptions>> options_
) {

	auto button = make_sptr<uikit::Button>();
	const auto& theming = getRuntimeTheming();

	::hg::move(options_).apply(
		Overloaded {
			[&](TextButtonCreateOptions& textOptions_) {

				switch (textOptions_.level) {
					case 0: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(0u, { .hover = true }));
						break;
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(1u, { .hover = true }));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(2u, { .hover = true }));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(3u, { .hover = true }));
						break;
					default: ;
				}

				/**/

				auto background = make_sptr<uikit::Paint>(ReflowClassList { "[Button] > background"sv }, nullptr);
				button->addChild(background);
				button->addChild(::hg::move(textOptions_.text));
			},
			[&](ImageButtonCreateOptions& imageOptions_) {

				switch (imageOptions_.level) {
					case 0: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(0u, { .hover = true }));
						break;
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(1u, { .hover = true }));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(2u, { .hover = true }));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(3u, { .hover = true }));
						break;
					default: ;
				}

				button->addChild(::hg::move(imageOptions_.image));
			},
			[&](ButtonCreateOptions& customOptions_) {

				switch (customOptions_.level) {
					case 0: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(0u, { .hover = true }));
						break;
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(1u, { .hover = true }));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(2u, { .hover = true }));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, theming->getStatedTheme(3u, { .hover = true }));
						break;
					default: ;
				}

				button->addChild(::hg::move(customOptions_.children));
			}
		}
	);

	return button;
}

uikit::Button::Button() = default;

uikit::Button::~Button() = default;

string uikit::Button::getTag() const noexcept {
	return std::format(R"(Button <{:#x}>)", reinterpret_cast<u64>(this));
}

void uikit::Button::setBaseTheme(mref<Opt<theming::Theme>> theme_) {

	bool effective = false;
	if (_themeStateMap.base.has_value()) {
		effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.base.value()));
	}

	_themeStateMap.base = ::hg::move(theme_);

	if (_themeStateMap.base.has_value()) {
		effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.base.value()));
	}

	/**/

	if (effective) {
		notifyContextChange();
	}
}

void uikit::Button::setStateTheme(InteractiveStateFlagBits interactiveBit_, mref<Opt<theming::Theme>> theme_) {

	bool effective = false;

	switch (interactiveBit_) {
		case InteractiveStateFlagBits::eFocus: {
			if (_state.isFocus() && _themeStateMap.focus.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.focus.value()));
			}

			_themeStateMap.focus = ::hg::move(theme_);

			if (_state.isFocus() && _themeStateMap.focus.has_value()) {
				effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.focus.value()));
			}
			break;
		}
		case InteractiveStateFlagBits::eHover: {
			if (_state.isHover() && _themeStateMap.hover.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.hover.value()));
			}

			_themeStateMap.hover = ::hg::move(theme_);

			if (_state.isHover() && _themeStateMap.hover.has_value()) {
				effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.hover.value()));
			}
			break;
		}
		case InteractiveStateFlagBits::eDisable: {
			if (/* TODO: */false && _themeStateMap.disabled.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.disabled.value()));
			}

			_themeStateMap.disabled = ::hg::move(theme_);

			if (/* TODO: */false && _themeStateMap.disabled.has_value()) {
				effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.disabled.value()));
			}
			break;
		}
		case InteractiveStateFlagBits::ePress: {
			if (_state.isPressed() && _themeStateMap.pressed.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.pressed.value()));
			}

			_themeStateMap.pressed = ::hg::move(theme_);

			if (_state.isPressed() && _themeStateMap.pressed.has_value()) {
				effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.pressed.value()));
			}
			break;
		}
		case InteractiveStateFlagBits::eSelected: {
			if (_state.isPressed() && _themeStateMap.selected.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.selected.value()));
			}

			_themeStateMap.selected = ::hg::move(theme_);

			if (_state.isPressed() && _themeStateMap.selected.has_value()) {
				effective |= getLocalContext().addLocalTheme(std::addressof(_themeStateMap.selected.value()));
			}
			break;
		}
		default: return;
	}

	if (effective) {
		notifyContextChange();
	}
}

Opt<ref<const theming::ThemeProvisioner>> uikit::Button::findParentProvisioner() const noexcept {
	return hasParent() ? parent()->cachedUpdateNearestProvisioner(false) : None;
}

Opt<ref<const theming::ThemeProvisioner>> uikit::Button::cachedUpdateNearestProvisioner(
	[[maybe_unused]] bool localInvalidate_
) noexcept {

	const auto parentProvisioner = findParentProvisioner();
	setLocalContextParent(parentProvisioner.has_value() ? Some(parentProvisioner->getContext()) : None);

	return Some(*this);
}

void uikit::Button::notifyContextChange() {

	/**
	 * Attention: We will take the naive straight-forward approach for now to brute-force cascade the updated state
	 *	to the whole sub-tree of this component. We may not expect too many sub-elements or nesting levels, as the
	 *	button should have a reasonable inclusive element size.
	 */
	Widget::cascadeContextChange(false);
}

EventResponse uikit::Button::invokeOnFocus(cref<FocusEvent> event_) {
	auto response = uikit::Stack::invokeOnFocus(event_);

	if (_themeStateMap.focus.has_value()) {
		if (getLocalContext().addLocalTheme(std::addressof(_themeStateMap.focus.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::invokeOnBlur(cref<FocusEvent> event_) {
	auto response = uikit::Stack::invokeOnBlur(event_);

	if (_themeStateMap.focus.has_value()) {
		if (getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.focus.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::invokeOnMouseButtonDown(ref<const MouseEvent> event_) {
	auto response = uikit::Stack::invokeOnMouseButtonDown(event_);

	if (_themeStateMap.pressed.has_value()) {
		if (getLocalContext().addLocalTheme(std::addressof(_themeStateMap.pressed.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::invokeOnMouseButtonUp(ref<const MouseEvent> event_) {
	auto response = uikit::Stack::invokeOnMouseButtonUp(event_);

	if (_themeStateMap.pressed.has_value()) {
		if (getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.pressed.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::invokeOnMouseEnter(ref<const MouseMoveEvent> event_) {
	auto response = uikit::Stack::invokeOnMouseEnter(event_);

	if (_themeStateMap.hover.has_value()) {
		if (getLocalContext().addLocalTheme(std::addressof(_themeStateMap.hover.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::invokeOnMouseLeave(ref<const MouseMoveEvent> event_) {
	auto response = uikit::Stack::invokeOnMouseLeave(event_);

	if (_themeStateMap.hover.has_value()) {
		if (getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.hover.value()))) {
			notifyContextChange();
		}
	}

	return response;
}
