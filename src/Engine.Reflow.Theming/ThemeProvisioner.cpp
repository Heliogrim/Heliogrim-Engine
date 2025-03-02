#include "ThemeProvisioner.hpp"

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

ref<ThemeContext> ThemeProvisioner::getLocalContext() {
	return _localContext;
}

void ThemeProvisioner::setLocalContextParent(Opt<ref<const ThemeContext>> parent_) {
	_localContext.parentContext = parent_;
}

ref<const ThemeContext> ThemeProvisioner::getContext() const {
	return _localContext;
}
