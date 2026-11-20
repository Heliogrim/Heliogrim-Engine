#pragma once

#include "ActionResolutionFactory.hpp"
#include "../Mutation.hpp"
#include "../Query.hpp"
#include "../Action/ActionTypeInfo.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	struct ActionTypeResolutionCache {
	public:
		using this_type = ActionTypeResolutionCache;

		using resolution_key = ActionTypeInfo;

	public:
		Vector<std::pair<resolution_key, UniquePtr<system::ActionResolutionFactory<Query>>>> queryActions {};
		Vector<std::pair<resolution_key, UniquePtr<system::ActionResolutionFactory<Mutation>>>> mutationActions {};
	};
}
