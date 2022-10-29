#include "Rock01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Rock01::Rock01() :
    StaticGeometryAsset(
        Rock01::auto_guid(),
        R"(resources\imports\obj\rock_01.obj)",
        49644ui64,
        49644ui64
    ) {}
