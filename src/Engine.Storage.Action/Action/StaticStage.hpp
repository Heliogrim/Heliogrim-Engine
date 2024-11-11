#pragma once

#include <Engine.Common/Managed/Rc.hpp>

#include "../Access/IoResourceAccessor.hpp"

namespace hg::engine::storage::system {
	/**
	 *
	 * @warning We are currently unable to use plane function pointers, as we cannot non-null instantiate them
	 *				only based on the captured type.
	 *
	 * @tparam StorageType_
	 * @tparam AcqFn_
	 * @tparam ResourceType_
	 * @tparam RelFn_
	 */
	template <
		class StorageType_,
		typename AcqFn_,
		class ResourceType_,
		typename RelFn_
	> requires (not std::is_pointer_v<AcqFn_>) && (not std::is_pointer_v<RelFn_>)
	struct StaticStage {
		Arci<StorageType_> storage;
		AcqFn_ acqFn;
		IoResourceAccessor<ResourceType_> access;
		RelFn_ relFn;
	};
}
