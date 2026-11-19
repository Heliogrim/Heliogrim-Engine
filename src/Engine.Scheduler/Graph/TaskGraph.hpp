#pragma once

#include <Engine.Common/__macro.hpp>

#include "StageDecl.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler::graph {
	struct GraphError {
		enum class Kind : u8 {
			eUnknownStage,
			eDuplicateStage,
			eCycle,
			eConflict
		};

		Kind kind;
		// the cycle in edge order, the two conflicting stages, or the one offender
		Vector<string> stages;
		Opt<AccessKey> key;
	};

	struct Graph {
		struct Node {
			string name;
			u16 predecessors;
			Vector<u16> successors;
		};

		Vector<Node> nodes;
		Vector<u16> order;
	};

	[[nodiscard]] Result<Graph, GraphError> build(std::span<const StageDecl> decls_);
}
