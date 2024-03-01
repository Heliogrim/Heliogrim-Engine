#include "Sphere.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::meshes;
using namespace hg;

Sphere::Sphere() :
    StaticGeometryAsset(
        clone(Sphere::unstable_auto_guid()),
        R"(resources\imports\obj\sphere.obj)",
        11520uLL,
        11520uLL
    ) {}
