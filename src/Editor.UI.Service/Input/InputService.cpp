#include "InputService.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

InputService::InputService() = default;

InputService::~InputService() = default;

bool InputService::addInputBindings(const StringView key_, mref<UniquePtr<InputBindingList>> bindings_) {
	const auto result = _bindingLists.emplace(key_, ::hg::move(bindings_));
	return result.second;
}

Opt<ref<const InputBindingList>> InputService::getInputBindings(ref<const String> key_) const noexcept {
	const auto it = _bindingLists.find(key_);
	return it != _bindingLists.end() ? Some(*it->second) : None;
}
