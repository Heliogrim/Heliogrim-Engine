#include "Config.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Asserts/Todo.hpp>

using namespace ::hg::engine;
using namespace ::hg;

/**/

[[nodiscard]] constexpr static cfg::ProviderId projectId(
	const std::pair<cfg::ProviderId, uptr<cfg::Provider>>& pair_
) noexcept {
	return pair_.first;
}

/**/

tl::expected<void, cfg::ConfigInitError> Config::init(mref<cfg::ConfigEntry> entry_) {
	if (_providers.empty()) {
		return tl::make_unexpected(cfg::ConfigInitError { "" });
	}

	auto& provider = *_providers.back().second;
	if (provider.contains(entry_.hashId)) {
		return tl::make_unexpected(cfg::ConfigInitError { "" });
	}

	provider.add(std::move(entry_));
	return {};
}

tl::expected<void, cfg::ConfigInitError> Config::init(
	mref<cfg::ConfigEntry> entry_,
	cfg::ProviderId providerId_
) {
	const auto provideIt = std::ranges::find(_providers, providerId_, projectId);
	if (provideIt == _providers.end()) {
		return tl::make_unexpected(cfg::ConfigInitError { "" });
	}

	auto& prov = *provideIt->second;
	if (prov.contains(entry_.hashId)) {
		return tl::make_unexpected(cfg::ConfigInitError { "" });
	}

	prov.add(std::move(entry_));
	return {};
}

tl::optional<cfg::ConfigValue> Config::get(cref<cfg::InternalConfigKey> key_) const noexcept {
	const auto end = _providers.crend();
	for (auto it = _providers.crbegin(); it != end; ++it) {
		if (it->second->contains(key_)) {
			return it->second->get(key_);
		}
	}
	return tl::nullopt;
}

tl::expected<void, cfg::ConfigUpdateError> Config::update(
	cref<cfg::InternalConfigKey> key_,
	mref<cfg::ConfigValue> next_
) noexcept {
	const auto end = _providers.crend();
	auto it = _providers.crbegin();
	for (; it != end; ++it) {
		if (it->second->contains(key_)) {
			break;
		}
	}

	if (it == end) {
		return tl::make_unexpected(cfg::ConfigUpdateError { "" });
	}

	if (not it->second->set(key_, std::move(next_))) {
		return tl::make_unexpected(cfg::ConfigUpdateError { "" });
	}

	return {};
}

bool Config::drop(cref<cfg::InternalConfigKey> key_) noexcept {
	::hg::todo_panic();
}

ref<cfg::Provider> Config::registerProvider(mref<cfg::ProviderId> providerId_, mref<uptr<cfg::Provider>> provider_) {
	::hg::assertrt(std::ranges::find(_providers, providerId_, projectId) == _providers.end());
	const auto& result = _providers.emplace_back(std::make_pair(providerId_, std::move(provider_)));
	return *result.second;
}

uptr<cfg::Provider> Config::overwriteProvider(mref<cfg::ProviderId> providerId_, mref<uptr<cfg::Provider>> next_) {

	if (auto found = std::ranges::find(_providers, providerId_, projectId); found != _providers.end()) {
		return std::exchange(found->second, std::move(next_));
	}

	_providers.emplace_back(std::make_pair(providerId_, std::move(next_)));
	return nullptr;
}

uptr<cfg::Provider> Config::unregisterProvider(
	mref<cfg::ProviderId> providerId_,
	[[maybe_unused]] cref<cfg::Provider> provider_
) {
	const auto iter = std::ranges::find(_providers, providerId_, projectId);
	uptr<cfg::Provider> prev = (iter != _providers.end()) ? nullptr : std::move(iter->second);
	_providers.erase(iter);
	return prev;
}
