#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::storage::system {
	template <class ActionType_, std::derived_from<ActionType_> InstantiationType_ = ActionType_>
	struct ActionResolutionFactory;

	template <typename StageType_>
	struct StageResolutionFactory;
}

namespace hg::engine::storage {
	struct StageKey;
	struct ResolvedStage;
	class StackCache;
}
