#include "ThemeContext.hpp"

#include "Algo/FirstOf.hpp"

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

std::span<const nmpt<const Theme>> ThemeContext::getLocalThemes() const noexcept {
	return localThemes;
}

bool ThemeContext::addLocalTheme(nmpt<const Theme> theme_) {
	if (std::ranges::contains(localThemes, theme_)) {
		return false;
	}

	localThemes.emplace_back(::hg::move(theme_));
	return true;
}

bool ThemeContext::dropLocalTheme(const nmpt<const Theme> theme_) {
	const auto where = std::ranges::find(localThemes, theme_);
	if (where == std::ranges::end(localThemes)) {
		return false;
	}

	localThemes.erase(where);
	return true;
}

Opt<ref<const ImageStyleAttributes>> ThemeContext::getImageStyle(StringView className_) const {

	const auto maybeLocal = FirstOf<Opt<ref<const ImageStyleAttributes>>>(
		localThemes,
		[&](const nmpt<const Theme>& theme_) {
			return theme_->getImageStyle(className_);
		}
	);

	if (maybeLocal != None) {
		return maybeLocal;
	}

	/**/

	if (parentContext != None) {
		return parentContext->getImageStyle(className_);
	}

	return None;
}

Opt<ref<const PaintStyleAttributes>> ThemeContext::getPaintStyle(StringView className_) const {

	const auto maybeLocal = FirstOf<Opt<ref<const PaintStyleAttributes>>>(
		localThemes,
		[&](const nmpt<const Theme>& theme_) {
			return theme_->getPaintStyle(className_);
		}
	);

	if (maybeLocal != None) {
		return maybeLocal;
	}

	/**/

	if (parentContext != None) {
		return parentContext->getPaintStyle(className_);
	}

	return None;
}

Opt<ref<const TextStyleAttributes>> ThemeContext::getTextStyle(StringView className_) const {

	const auto maybeLocal = FirstOf<Opt<ref<const TextStyleAttributes>>>(
		localThemes,
		[&](const nmpt<const Theme>& theme_) {
			return theme_->getTextStyle(className_);
		}
	);

	if (maybeLocal != None) {
		return maybeLocal;
	}

	/**/

	if (parentContext != None) {
		return parentContext->getTextStyle(className_);
	}

	return None;
}

Opt<ref<const BoxLayoutAttributes>> ThemeContext::getBoxLayout(StringView className_) const {

	const auto maybeLocal = FirstOf<Opt<ref<const BoxLayoutAttributes>>>(
		localThemes,
		[&](const nmpt<const Theme>& theme_) {
			return theme_->getBoxLayout(className_);
		}
	);

	if (maybeLocal != None) {
		return maybeLocal;
	}

	/**/

	if (parentContext != None) {
		return parentContext->getBoxLayout(className_);
	}

	return None;
}
