#include "WoodenBucket01.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenBucket01::WoodenBucket01() :
    StaticGeometryAsset(
        WoodenBucket01::auto_guid(),
        R"(resources\imports\obj\wooden_bucket_01.obj)",
        15348ui64,
        15348ui64
    ) {}
