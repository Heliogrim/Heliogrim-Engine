#pragma once

#include "../ModelComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg {
	class ShapeGeometryComponent :
		public InheritMeta<ShapeGeometryComponent, ModelComponent> {
	public:
		using this_type = ShapeGeometryComponent;
		using underlying_type = ModelComponent;

	public:
		using InheritMeta::InheritMeta;
	};
}
