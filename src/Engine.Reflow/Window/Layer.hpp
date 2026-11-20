#pragma once

#include "../Widget/Layer/Host.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	using Layer = std::pair<sptr<Window>, wptr<Host>>;
}
