#pragma once

#include <Engine.Common/__macro.hpp>

#include "NodeOutputSpec.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::graphics::graph {
	class NodeOutput {
	public:
		using this_type = NodeInput;

	public:
		string _uniqueName;
		NodeOutputSpec _spec;
	};
}
