#include "Cannon01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Cannon01::Cannon01() :
    StaticGeometryAsset(
        Cannon01::unstable_auto_guid(),
        R"(resources\imports\fbx\cannon_01.fbx)",
        124332ui64,
        124332ui64
    ) {}
