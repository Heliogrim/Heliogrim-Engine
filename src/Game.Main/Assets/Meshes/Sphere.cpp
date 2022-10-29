#include "Sphere.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Sphere::Sphere() :
    StaticGeometryAsset(
        Sphere::auto_guid(),
        R"(resources\imports\obj\sphere.obj)",
        11520ui64,
        11520ui64
    ) {}
