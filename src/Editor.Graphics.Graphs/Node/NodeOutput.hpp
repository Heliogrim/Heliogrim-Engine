#pragma once

#include <Engine.Common/String.hpp>

#include "__fwd.hpp"
#include "NodeOutputSpec.hpp"

namespace hg::editor::graphics::graph {
	class NodeOutput {
	public:
		using this_type = NodeInput;

	public:
		string _uniqueName;
		NodeOutputSpec _spec;
	};
}
