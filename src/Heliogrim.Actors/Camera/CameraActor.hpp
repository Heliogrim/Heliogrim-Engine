#pragma once

#include <Engine.ACS/Actor/Actor.hpp>

#include <Engine.Graphics.Components/Camera/CameraComponent.hpp>

namespace hg {
	class CameraActor :
		public InheritMeta<CameraActor, Actor> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = CameraActor;
		using underlying_type = Actor;

		constexpr static type_id typeId { "CameraActor"_typeId };

	public:
		CameraActor(cref<ActorInitializer> initializer_);

	private:
		ptr<CameraComponent> _cameraComponent;
		ptr<HierarchyComponent> _mountComponent;

	public:
		[[nodiscard]] nmpt<CameraComponent> getCameraComponent() const noexcept;
	};
}
