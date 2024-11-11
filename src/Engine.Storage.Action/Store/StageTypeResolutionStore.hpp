#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "StageResolutionFactory.hpp"
#include "../Action/StageTypeInfo.hpp"

namespace hg::engine::storage {
	struct StageTypeResolutionStore {
	public:
		using this_type = StageTypeResolutionStore;

		/**/

		template <typename StageType_>
		using resolution_entry_type = std::pair<StageTypeInfo, UniquePtr<system::StageResolutionFactory<StageType_>>>;

	public:
		Vector<resolution_entry_type<system::DynamicStage>> dynamicResolved;
		Vector<resolution_entry_type<system::MorphStageType>> morphingResolved;
		Vector<resolution_entry_type<system::ComposeStageType>> composingResolved;
	};
}
