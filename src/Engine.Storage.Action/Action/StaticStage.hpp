#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Access/IoResourceAccessor.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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
