#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "__fwd.hpp"

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
