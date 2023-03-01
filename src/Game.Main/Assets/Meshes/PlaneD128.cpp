#include "PlaneD128.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

PlaneD128::PlaneD128() :
    StaticGeometryAsset(
        PlaneD128::unstable_auto_guid(),
        R"(resources\imports\obj\plane_d128.obj)",
        98304ui64,
        98304ui64
    ) {}
