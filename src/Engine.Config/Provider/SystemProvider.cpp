module;

#include <Engine.Asserts/Asserts.hpp>

module hgrim.config.providers;

using namespace ::hg::engine::cfg;
using namespace ::hg;

void SystemProvider::add(mref<ConfigEntry> entry_) {
	const auto key = entry_.hashId;
	_props.emplace(key, std::move(entry_));
}

bool SystemProvider::contains(InternalConfigKey key_) const noexcept {
	return _props.contains(key_);
}

ConfigValue SystemProvider::get(InternalConfigKey key_) const {
	::hg::assertd([this, key_]() { return _props.contains(key_); });
	return _props.at(key_).holder;
}

bool SystemProvider::set(InternalConfigKey key_, mref<ConfigValue> next_) noexcept {

	const auto it = _props.find(key_);
	if (it == _props.end()) {
		return false;
	}

	if (it->second.holder.index() != next_.index()) {
		return false;
	}

	it->second.holder = std::move(next_);
	return true;
}

bool SystemProvider::remove(InternalConfigKey key_) {
	return _props.erase(key_) != 0;
}

bool SystemProvider::isConfigType(InternalConfigKey key_, type_id typeId_) const noexcept {
	if (const auto it = _props.find(key_); it != _props.end()) {
		return it->second.typeId == typeId_;
	}
	return false;
}
