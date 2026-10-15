#pragma once

#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "../ModelComponent.hpp"

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
