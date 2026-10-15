#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Math/Location.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

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
		Opt<ActorGuid> _actorGuid;
	};
}
