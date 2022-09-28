#include "WoodenBucket01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenBucket01::WoodenBucket01() :
    StaticGeometryAsset(
        WoodenBucket01::auto_guid(),
        R"(resources\assets\geometry\wooden_bucket_01.obj)",
        2872ui64,
        2872ui64
    ) {}
