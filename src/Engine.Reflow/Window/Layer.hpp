#pragma once

#include "__fwd.hpp"
#include "LayerBand.hpp"
#include "../Widget/Layer/Host.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	struct Layer {
		wptr<Host> host;
		LayerBand band;
	};
}
