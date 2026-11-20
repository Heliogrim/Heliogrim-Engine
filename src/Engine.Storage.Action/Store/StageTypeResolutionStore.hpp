#pragma once

#include "../Action/StageTypeInfo.hpp"
#include "StageResolutionFactory.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
