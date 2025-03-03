#include "Button.hpp"

#include <format>
#include <Engine.Common/Functional/Lambda.hpp>
#include <Engine.Reflow/ReflowClassList.hpp>

#include "../Atom/Paint.hpp"
#include "../Atom/Layout/BoxLayout.hpp"
#include "../Theme/Themes.hpp"

using namespace hg::engine::reflow;
using namespace hg;

SharedPtr<uikit::Button> uikit::makeButton(
	mref<Variant<TextButtonCreateOptions, ImageButtonCreateOptions, ButtonCreateOptions>> options_
) {

	auto button = make_sptr<uikit::Button>();

	::hg::move(options_).apply(
		Overloaded {
			[&](TextButtonCreateOptions& textOptions_) {

				switch (textOptions_.level) {
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl1_hover_theme()));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl2_hover_theme()));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl3_hover_theme()));
						break;
					default: ;
				}

				/**/

				auto background = make_sptr<uikit::Paint>(ReflowClassList { "[Button] > background"sv }, nullptr);
				button->addChild(background);

				auto wrapper = make_sptr<uikit::BoxLayout>();
				wrapper->getLayoutAttributes().update<attr::BoxLayout::padding>(Padding { 8.F });

				wrapper->setChild(::hg::move(textOptions_.text));
				button->addChild(::hg::move(wrapper));
			},
			[&](ImageButtonCreateOptions& imageOptions_) {

				switch (imageOptions_.level) {
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl1_hover_theme()));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl2_hover_theme()));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl3_hover_theme()));
						break;
					default: ;
				}

				button->addChild(::hg::move(imageOptions_.image));
			},
			[&](ButtonCreateOptions& customOptions_) {

				switch (customOptions_.level) {
					case 1: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl1_hover_theme()));
						break;
					case 2: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl2_hover_theme()));
						break;
					case 3: button->setStateTheme(InteractiveStateFlagBits::eHover, Some(uikit::generate_lvl3_hover_theme()));
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
			if (_state.isDisabled() && _themeStateMap.disabled.has_value()) {
				effective |= getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.disabled.value()));
			}

			_themeStateMap.disabled = ::hg::move(theme_);

			if (_state.isDisabled() && _themeStateMap.disabled.has_value()) {
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

EventResponse uikit::Button::onMouseButtonDown(cref<MouseEvent> event_) {
	auto response = uikit::Stack::onMouseButtonDown(event_);

	if (_themeStateMap.pressed.has_value()) {
		if (getLocalContext().addLocalTheme(std::addressof(_themeStateMap.pressed.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::onMouseButtonUp(cref<MouseEvent> event_) {
	auto response = uikit::Stack::onMouseButtonUp(event_);

	if (_themeStateMap.pressed.has_value()) {
		if (getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.pressed.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::onMouseEnter(cref<MouseMoveEvent> event_) {
	auto response = uikit::Stack::onMouseEnter(event_);

	if (_themeStateMap.hover.has_value()) {
		if (getLocalContext().addLocalTheme(std::addressof(_themeStateMap.hover.value()))) {
			notifyContextChange();
		}
	}

	return response;
}

EventResponse uikit::Button::onMouseLeave(cref<MouseMoveEvent> event_) {
	auto response = uikit::Stack::onMouseLeave(event_);

	if (_themeStateMap.hover.has_value()) {
		if (getLocalContext().dropLocalTheme(std::addressof(_themeStateMap.hover.value()))) {
			notifyContextChange();
		}
	}

	return response;
}
