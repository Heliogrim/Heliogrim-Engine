#include "WoodenPier01Planks.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenPier01Planks::WoodenPier01Planks() :
    StaticGeometryAsset(
        WoodenPier01Planks::auto_guid(),
        R"(resources\assets\geometry\wooden_pier_01_planks.obj)",
        16536ui64,
        16536ui64
    ) {}
