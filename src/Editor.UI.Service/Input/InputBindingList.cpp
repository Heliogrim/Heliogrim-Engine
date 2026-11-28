#include "InputBindingList.hpp"

import Editor.Action;
import Engine.Common;
import Engine.Diagnostics;

using namespace hg::editor::ui::service;
using namespace hg;

bool InputBindingList::addBinding(mref<InputBinding> binding_) {

	if (std::ranges::any_of(
		_bindings,
		[&binding_](ref<const InputBinding> stored_) {
			return std::ranges::equal(stored_.binding, binding_.binding);
		}
	)) {
		return false;
	}

	_bindings.emplace_back(::hg::move(binding_));
	return true;
}

Opt<Result<Arci<editor::Action>, engine::diag::Diagnostic>> InputBindingList::invoke(ref<const InlineAutoArray<u32, 4>> keys_) const {
	auto binding = std::ranges::find_if(
		_bindings,
		[&keys_](ref<const InputBinding> binding_) {
			return std::ranges::equal(binding_.binding, keys_) && (binding_.condition == nullptr || binding_.condition());
		}
	);

	if (binding == _bindings.end()) {
		return None;
	}

	return Some(binding->invoke());
}
