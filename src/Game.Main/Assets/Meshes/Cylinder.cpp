#include "Cylinder.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Cylinder::Cylinder() :
    StaticGeometryAsset(
        Cylinder::auto_guid()
    ) {}
