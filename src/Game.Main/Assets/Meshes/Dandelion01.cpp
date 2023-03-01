#include "Dandelion01.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Dandelion01::Dandelion01() :
    StaticGeometryAsset(
        Dandelion01::unstable_auto_guid(),
        R"(resources\imports\obj\dandelion_01.obj)",
        70'626ui64,
        70'626ui64
    ) {}
