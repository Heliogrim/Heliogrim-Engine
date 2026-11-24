#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Graphics.Components;
#endif

namespace hg {
	class CameraActor :
		public InheritArchetype<CameraActor, Actor> {
	public:
		using this_type = CameraActor;
		constexpr static type_id typeId { "CameraActor"_typeId };

		using layout = engine::acs::ActorLayout<
			engine::acs::Slot<"mount", HierarchyComponent, engine::acs::Children<
				engine::acs::Slot<"camera", CameraComponent>>>>;

	public:
		explicit CameraActor(cref<ActorInitializer> initializer_);

	public:
		[[nodiscard]] nmpt<CameraComponent> getCameraComponent() noexcept;
	};
}
