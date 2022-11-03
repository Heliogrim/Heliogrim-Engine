#include "Cerberus.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Cerberus::Cerberus() :
    StaticGeometryAsset(
        Cerberus::unstable_auto_guid(),
        R"(resources\imports\gltf\cerberus.gltf)",
        100623ui64,
        100623ui64
    ) {}
