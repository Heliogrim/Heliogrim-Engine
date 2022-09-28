#include "Cylinder.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Cylinder::Cylinder() :
    StaticGeometryAsset(
        Cylinder::auto_guid(),
        R"(resources\assets\geometry\cylinder.obj)",
        1140ui64,
        1140ui64
    ) {}
