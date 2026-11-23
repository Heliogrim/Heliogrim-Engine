#include "Themes.hpp"

import Engine.Core;
import Engine.Reflow;

using namespace hg::engine::reflow;
using namespace hg;

Opt<ref<const theming::Theming>> uikit::getRuntimeTheming() noexcept {
	const auto reflowModule = Engine::getEngine()->getModules().getSubModule(ReflowDepKey);
	return reflowModule != nullptr ? Some(static_cast<ref<const Reflow>>(*reflowModule).getTheming()) : None;
}
