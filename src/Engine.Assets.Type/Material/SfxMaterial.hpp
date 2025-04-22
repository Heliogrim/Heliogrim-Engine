#pragma once
#include "Material.hpp"

namespace hg::engine::assets {
	class SfxMaterial :
		public InheritMeta<SfxMaterial, Material> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "SfxMaterial"_typeId };
	};
}
