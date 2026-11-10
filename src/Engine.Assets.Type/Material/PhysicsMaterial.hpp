#pragma once

#include "Material.hpp"

namespace hg::engine::assets {
	class PhysicsMaterial :
		public InheritMeta<PhysicsMaterial, Material> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "PhysicsMaterial"_typeId };
	};
}
