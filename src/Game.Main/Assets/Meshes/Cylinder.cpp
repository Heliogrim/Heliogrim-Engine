#include "Cylinder.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::meshes;
using namespace hg;

Cylinder::Cylinder() :
	StaticGeometryAsset(
		clone(Cylinder::unstable_auto_guid()),
		R"(resources\imports\obj\cylinder.obj)",
		1140uLL,
		1140uLL
	) {}
