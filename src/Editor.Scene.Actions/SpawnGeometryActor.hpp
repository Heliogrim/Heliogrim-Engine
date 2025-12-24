#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Math/Location.hpp>
#include <Heliogrim/TypedAssetGuid.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

namespace hg::editor::scene::actions {
	class SpawnGeometryActor :
		public DelegateAction {
	public:
		SpawnGeometryActor(TypedAssetGuid<engine::assets::StaticGeometry> assetGuid_, mref<math::Location> location_);

		~SpawnGeometryActor() override;

	private:
		Opt<ActorGuid> _actorGuid;
	};
}