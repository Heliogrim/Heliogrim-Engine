#include "Rock01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Rock01::Rock01() :
    StaticGeometryAsset(
        Rock01::auto_guid(),
        R"(resources\assets\geometry\rock_01.obj)",
        6000ui64,
        6000ui64
    ) {}
