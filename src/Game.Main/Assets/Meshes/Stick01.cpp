#include "Stick01.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Stick01::Stick01() :
    StaticGeometryAsset(
        Stick01::unstable_auto_guid(),
        R"(resources\imports\obj\stick_01.obj)",
        150'000ui64,
        150'000ui64
    ) {}
