#pragma once
#include <Engine.ECS/ComponentTypeId.hpp>

#include "SceneComponent.hpp"

namespace ember::engine::ecs::subsystem {

	class SkeletalMeshComponent:
		public SceneComponent {
	public:
		constexpr static component_type_id type_id { "StaticMeshComponent"_typeId };

	public:
		SkeletalMeshComponent() noexcept = default;
	};
}
