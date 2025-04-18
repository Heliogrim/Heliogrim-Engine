#include "Card.hpp"

#include <format>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow.Theming/Theming.hpp>

#include "../Themes.hpp"
#include "../Atom/Paint.hpp"

using namespace hg::engine::reflow;
using namespace hg;

/**/

uikit::Card::Card() = default;

uikit::Card::~Card() = default;

string uikit::Card::getTag() const noexcept {
	return std::format(R"(Card <{:#x}>)", reinterpret_cast<u64>(this));
}

ref<const Opt<theming::Theme>> uikit::Card::getCardTheme() const noexcept {
	return _theme;
}

void uikit::Card::setCardTheme(mref<Opt<theming::Theme>> theme_) {

	bool effective = false;
	if (_theme.has_value()) {
		effective |= getLocalContext().dropLocalTheme(std::addressof(_theme.value()));
	}

	_theme = ::hg::move(theme_);

	if (_theme.has_value()) {
		effective |= getLocalContext().addLocalTheme(std::addressof(_theme.value()));
	}

	/**/

	if (effective) {
		cascadeContextChange(false);
	}
}

Opt<ref<const theming::ThemeProvisioner>> uikit::Card::findParentProvisioner() const noexcept {
	return hasParent() ? parent()->cachedUpdateNearestProvisioner(false) : None;
}

Opt<ref<const theming::ThemeProvisioner>> uikit::Card::cachedUpdateNearestProvisioner(bool localInvalidate_) noexcept {

	const auto parentProvisioner = findParentProvisioner();
	setLocalContextParent(parentProvisioner.has_value() ? Some(parentProvisioner->getContext()) : None);

	return Some(*this);
}

/**/

SharedPtr<uikit::Card> uikit::makeCard(mref<CardCreateOptions> options_) {

	auto card = make_sptr<Card>();
	const auto& theming = getRuntimeTheming();

	switch (options_.level) {
		case 1: card->setCardTheme(theming->getStatedTheme(1u, {}));
			break;
		case 2: card->setCardTheme(theming->getStatedTheme(2u, {}));
			break;
		case 3: card->setCardTheme(theming->getStatedTheme(3u, {}));
			break;
		default: ;
	}

	/**/

	auto background = make_sptr<uikit::Paint>(ReflowClassList { "[Card] > background"sv }, nullptr);
	card->addChild(::hg::move(background));

	if (options_.children != nullptr) {
		card->addChild(::hg::move(options_.children));
	}

	return card;
}
