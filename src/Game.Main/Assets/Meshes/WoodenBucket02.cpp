#include "WoodenBucket02.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenBucket02::WoodenBucket02() :
    StaticGeometryAsset(
        WoodenBucket02::auto_guid(),
        R"(resources\assets\geometry\wooden_bucket_02.obj)",
        4164ui64,
        4164ui64
    ) {}
