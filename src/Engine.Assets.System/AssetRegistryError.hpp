#pragma once

#include "Error/IndexFailure.hpp"
#include "Error/IndexSoftFailure.hpp"
#include "Error/RepositoryFailure.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets {
	class AssetRegistryError final :
		public Variant<system::IndexFailure, system::IndexSoftFailure, system::RepositoryFailure> {
	public:
		using Variant::this_type;
	};
}
