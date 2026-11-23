#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Common;
#endif

namespace hg::editor::scene::actions {
	class SpawnShapeActor :
		public DelegateAction {
	public:
		enum class ShapeType {
			eBox, eSphere
		};

	public:
		SpawnShapeActor(ShapeType shapeType_, mref<math::Location> location_);

		~SpawnShapeActor() override;

	private:
		Opt<ActorRef<>> _actorRef;
	};
}
