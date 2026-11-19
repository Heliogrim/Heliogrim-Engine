#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::theming {
	class Theming;
}

namespace hg::engine::reflow::uikit {
	extern Opt<ref<const theming::Theming>> getRuntimeTheming() noexcept;
}
