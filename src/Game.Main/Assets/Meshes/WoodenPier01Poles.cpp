#include "WoodenPier01Poles.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

WoodenPier01Poles::WoodenPier01Poles() :
    StaticGeometryAsset(
        WoodenPier01Poles::unstable_auto_guid(),
        R"(resources\imports\obj\wooden_pier_01_poles.obj)",
        7380ui64,
        7380ui64
    ) {}
