module;
module Engine.Diagnostics;

import std;

import :Sink;

using namespace hg::engine::diag;
using namespace hg;

void CollectingSink::emit(mref<Diagnostic> diagnostic_) {
	_hasErrors = _hasErrors || diagnostic_.severity() == Severity::eError;
	_items.push_back(std::move(diagnostic_));
}

bool CollectingSink::hasErrors() const noexcept {
	return _hasErrors;
}

ref<const Diagnostics> CollectingSink::items() const noexcept {
	return _items;
}

Diagnostics CollectingSink::take() {
	_hasErrors = false;
	return std::exchange(_items, Diagnostics {});
}
