#include "Rock01.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Rock01::Rock01() :
    StaticGeometryAsset(
        Rock01::unstable_auto_guid(),
        R"(resources\imports\obj\rock_01.obj)",
        49644ui64,
        49644ui64
    ) {}
