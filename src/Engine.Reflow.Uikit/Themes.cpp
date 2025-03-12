#include "Themes.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Module/Modules.hpp>
#include <Engine.Reflow/Reflow.hpp>
#include <Engine.Reflow/Module/Reflow.hpp>

using namespace hg::engine::reflow;
using namespace hg;

Opt<ref<const theming::Theming>> uikit::getRuntimeTheming() noexcept {
	const auto reflowModule = Engine::getEngine()->getModules().getSubModule(ReflowDepKey);
	return reflowModule != nullptr ? Some(static_cast<ref<const Reflow>>(*reflowModule).getTheming()) : None;
}
