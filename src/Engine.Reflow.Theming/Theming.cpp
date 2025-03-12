#include "Theming.hpp"

#include <format>

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

/**/

static auto statedToKey(const u8 level_, const ThemeStateOptions states_) noexcept {

	const auto flag = std::bit_cast<u8>(states_);

	if (flag == 0u) {
		const auto source = std::format(R"([Level-{}])", level_);
		return hash::fnv1a(source);
	}

	if (states_.disabled) {
		const auto source = std::format(R"(:disable([Level-{}]))", level_);
		return hash::fnv1a(source);
	}

	if (states_.dragged) {
		const auto source = std::format(R"(:drag([Level-{}]))", level_);
		return hash::fnv1a(source);
	}

	if (states_.pressed) {
		const auto source = std::format(R"(:press([Level-{}]))", level_);
		return hash::fnv1a(source);
	}

	if (states_.hover) {
		const auto source = std::format(R"(:hover([Level-{}]))", level_);
		return hash::fnv1a(source);
	}

	if (states_.focus) {
		const auto source = std::format(R"(:focus([Level-{}]))", level_);
		return hash::fnv1a(source);
	}

	std::unreachable();
}

/**/

Theming::Theming() noexcept = default;

Theming::~Theming() noexcept = default;

Opt<ref<const Theme>> Theming::getBaseTheme() const noexcept {
	constexpr static auto hashKey = hash::fnv1a("[Base]"sv);
	const auto it = _themes.find(hashKey);
	return it != _themes.end() ? Some(*it->second) : None;
}

Opt<ref<const Theme>> Theming::getTheme(StringView key_) const noexcept {
	const auto hashKey = hash::fnv1a(key_);
	const auto it = _themes.find(hashKey);
	return it != _themes.end() ? Some(*it->second) : None;
}

Opt<ref<const Theme>> Theming::getStatedTheme(u8 level_, ThemeStateOptions states_) const noexcept {
	const auto hashKey = statedToKey(level_, states_);
	const auto it = _themes.find(hashKey);
	return it != _themes.end() ? Some(*it->second) : None;
}

void Theming::storeBaseTheme(mref<Theme> theme_) {
	constexpr static auto hashKey = hash::fnv1a("[Base]"sv);
	const auto it = _themes.find(hashKey);

	if (it != _themes.end()) {
		*it->second = ::hg::move(theme_);
		return;
	}

	_themes.insert(std::make_pair(hashKey, make_uptr<Theme>(::hg::move(theme_))));
}

void Theming::storeTheme(StringView key_, mref<Theme> theme_) {
	const auto hashKey = hash::fnv1a(key_);
	const auto it = _themes.find(hashKey);

	if (it != _themes.end()) {
		*it->second = ::hg::move(theme_);
		return;
	}

	_themes.insert(std::make_pair(hashKey, make_uptr<Theme>(::hg::move(theme_))));
}

void Theming::storeStatedTheme(u8 level_, ThemeStateOptions states_, mref<Theme> theme_) {
	const auto hashKey = statedToKey(level_, states_);
	const auto it = _themes.find(hashKey);

	if (it != _themes.end()) {
		*it->second = ::hg::move(theme_);
		return;
	}

	_themes.insert(std::make_pair(hashKey, make_uptr<Theme>(::hg::move(theme_))));
}
