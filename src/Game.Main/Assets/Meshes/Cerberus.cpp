#include "Cerberus.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

Cerberus::Cerberus() :
    StaticGeometryAsset(
        Cerberus::auto_guid(),
        R"(resources\assets\geometry\cerberus.gltf)",
        100623ui64,
        100623ui64
    ) {}
