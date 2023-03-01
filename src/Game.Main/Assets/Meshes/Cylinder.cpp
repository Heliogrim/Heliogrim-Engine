#include "Cylinder.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Cylinder::Cylinder() :
    StaticGeometryAsset(
        Cylinder::unstable_auto_guid(),
        R"(resources\imports\obj\cylinder.obj)",
        1140ui64,
        1140ui64
    ) {}
