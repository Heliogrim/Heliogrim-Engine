#include "PlaneD128.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

PlaneD128::PlaneD128() :
    StaticGeometryAsset(
        PlaneD128::auto_guid(),
        R"(resources\imports\obj\plane_d128.obj)",
        98304ui64,
        98304ui64
    ) {}
