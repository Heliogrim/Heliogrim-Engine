#pragma once

#include "Geometry.hpp"

namespace hg::engine::assets {
	class SkeletalGeometry :
		public InheritMeta<SkeletalGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static asset_type_id typeId { "SkeletalGeometry"_typeId };
	};
}
