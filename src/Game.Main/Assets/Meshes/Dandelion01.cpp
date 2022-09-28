#include "Dandelion01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Dandelion01::Dandelion01() :
    StaticGeometryAsset(
        Dandelion01::auto_guid(),
        R"(resources\assets\geometry\dandelion_01.obj)",
        70'626ui64,
        70'626ui64
    ) {}
