#pragma once

#include <Engine.Common/Variant.hpp>

#include "Error/IndexFailure.hpp"
#include "Error/IndexSoftFailure.hpp"
#include "Error/RepositoryFailure.hpp"

namespace hg::engine::assets {
	class AssetRegistryError final :
		public Variant<system::IndexFailure, system::IndexSoftFailure, system::RepositoryFailure> {
	public:
		using Variant::this_type;
	};
}
