#pragma once

#include <concepts>

namespace hg::engine::storage::system {
	template <class ActionType_, std::derived_from<ActionType_> InstantiationType_ = ActionType_>
	struct ActionResolutionFactory;

	template <typename StageType_>
	struct StageResolutionFactory;
}
