#include "Cannon01.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Cannon01::Cannon01() :
    StaticGeometryAsset(
        Cannon01::unstable_auto_guid(),
        R"(resources\imports\fbx\cannon_01.fbx)",
        124332ui64,
        124332ui64
    ) {}
