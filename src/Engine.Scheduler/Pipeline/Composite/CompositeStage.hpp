#pragma once

#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>

#include "CompositeDependency.hpp"

namespace hg::engine::scheduler {
	struct CompositeStage {
		const non_owning_rptr<const class Stage> stage;
		CompactArray<non_owning_rptr<const class PipelineStage>> pipelineStages;

		u64 generation;

		CompactSet<CompositeDependency> dependencies;

	public:
		[[nodiscard]] bool ready() const noexcept {
			for (const auto& entry : dependencies) {
				if (entry.requiredGeneration > entry.stage->generation) {
					return false;
				}
			}

			return true;
		}
	};
}