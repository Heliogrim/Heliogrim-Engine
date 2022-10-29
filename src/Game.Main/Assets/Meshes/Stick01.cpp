#include "Stick01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Stick01::Stick01() :
    StaticGeometryAsset(
        Stick01::auto_guid(),
        R"(resources\imports\obj\stick_01.obj)",
        150'000ui64,
        150'000ui64
    ) {}
