#pragma once
#include <Heliogrim/StaticGeometryAsset.hpp>

namespace hg::game::assets::meshes {
	class PlaneD128 :
		public StaticGeometryAsset,
		public AssetAutoGuid<PlaneD128> {
	public:
		PlaneD128();
	};
}
