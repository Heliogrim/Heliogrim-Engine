#include "Sphere.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Sphere::Sphere() :
    StaticGeometryAsset(
        Sphere::auto_guid()
    ) {}
