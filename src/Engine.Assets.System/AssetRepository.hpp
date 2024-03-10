#pragma once

#include "__fwd.hpp"
#include <Engine.Common/Sal.hpp>

namespace hg::engine::assets::system {
	class __declspec(novtable) AssetRepository {
	public:
		using this_type = AssetRepository;

	protected:
		AssetRepository() noexcept = default;

	public:
		virtual ~AssetRepository() = default;

	public:
		[[nodiscard]] virtual bool destroyAsset(_In_ mref<non_owning_rptr<Asset>> asset_) = 0;
	};
}
