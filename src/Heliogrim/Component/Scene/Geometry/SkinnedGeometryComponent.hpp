#pragma once
#include "SkeletalGeometryComponent.hpp"

namespace hg {
	class SkinnedGeometryComponent :
		public InheritMeta<SkinnedGeometryComponent, SkeletalGeometryComponent> {
	public:
		using this_type = SkinnedGeometryComponent;
		using underlying_type = SkeletalGeometryComponent;

		constexpr static ComponentTypeId typeId { "SkinnedGeometryComponent"_typeId };

	public:
	};
}
