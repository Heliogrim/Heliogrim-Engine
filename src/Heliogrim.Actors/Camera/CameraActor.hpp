#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Graphics.Components;
#endif

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
