#pragma once

#include <Engine.Common/__macro.hpp>

#include "../__fwd.hpp"
#include "LinkMode.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
	void LinkNodes(
		mref<nmpt<Node>> predecessor_,
		mref<smr<Node>> successor_,
		LinkMode mode_ = LinkMode::eInclusive
	);
}
