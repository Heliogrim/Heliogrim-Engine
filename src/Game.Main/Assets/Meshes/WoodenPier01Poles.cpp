#include "WoodenPier01Poles.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenPier01Poles::WoodenPier01Poles() :
    StaticGeometryAsset(
        WoodenPier01Poles::auto_guid(),
        R"(resources\assets\geometry\wooden_pier_01_poles.obj)",
        7380ui64,
        7380ui64
    ) {}
