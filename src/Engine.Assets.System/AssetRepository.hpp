#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/__macro.hpp>

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets::system {
	class macro_novtable AssetRepository {
	public:
		using this_type = AssetRepository;

	protected:
		AssetRepository() noexcept = default;

	public:
		virtual ~AssetRepository() = default;

	public:
		[[nodiscard]] virtual bool destroyAsset(_In_ mref<nmpt<Asset>> asset_) = 0;
	};
}
