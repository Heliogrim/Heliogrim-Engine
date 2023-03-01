#include "WoodenPier01Planks.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

WoodenPier01Planks::WoodenPier01Planks() :
    StaticGeometryAsset(
        WoodenPier01Planks::unstable_auto_guid(),
        R"(resources\imports\obj\wooden_pier_01_planks.fbx)",
        16536ui64,
        16536ui64
    ) {}
