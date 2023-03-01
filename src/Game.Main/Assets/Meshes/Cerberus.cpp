#include "Cerberus.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

Cerberus::Cerberus() :
    StaticGeometryAsset(
        Cerberus::unstable_auto_guid(),
        R"(resources\imports\gltf\cerberus.gltf)",
        100623ui64,
        100623ui64
    ) {}
