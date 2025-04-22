#pragma once

#include "Material.hpp"

namespace hg::engine::assets {
	class PfxMaterial :
		public InheritMeta<PfxMaterial, Material> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "PfxMaterial"_typeId };
	};
}
