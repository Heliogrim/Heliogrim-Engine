#pragma once

#include "Geometry.hpp"

namespace hg::engine::assets {
	class SkeletalGeometry :
		public InheritMeta<SkeletalGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "SkeletalGeometry"_typeId };
	};
}
