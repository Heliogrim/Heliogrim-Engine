#include "Sphere.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Sphere::Sphere() :
    StaticGeometryAsset(
        Sphere::unstable_auto_guid(),
        R"(resources\imports\obj\sphere.obj)",
        11520ui64,
        11520ui64
    ) {}
