#pragma once

#include <span>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "StageDecl.hpp"
#include "../Task/TaskMask.hpp"

namespace hg::engine::scheduler::graph {
	struct GraphError {
		enum class Kind : u8 {
			eUnknownStage,
			eDuplicateStage,
			eLaneMismatch,
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
			Lane lane;
			u16 predecessors;
			Vector<u16> successors;
		};

		Vector<Node> nodes;
		Vector<u16> order;
	};

	[[nodiscard]] Result<Graph, GraphError> build(std::span<const StageDecl> decls_);

	[[nodiscard]] task::TaskMask toMask(Lane lane_) noexcept;
}
