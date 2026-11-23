#pragma once

#include <Editor.Action/Action/DelegateAction.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Assets.Type;
import Engine.Common;
#endif

namespace hg::editor::scene::actions {
	class SpawnGeometryActor :
		public DelegateAction {
	public:
		SpawnGeometryActor(TypedAssetGuid<engine::assets::StaticGeometry> assetGuid_, mref<math::Location> location_);

		~SpawnGeometryActor() override;

	private:
		Opt<ActorRef<>> _actorRef;
	};
}
