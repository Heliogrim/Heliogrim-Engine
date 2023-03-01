#include "WoodenBucket01.hpp"

using namespace hg::game::assets::meshes;
using namespace hg;

WoodenBucket01::WoodenBucket01() :
    StaticGeometryAsset(
        WoodenBucket01::unstable_auto_guid(),
        R"(resources\imports\obj\wooden_bucket_01.obj)",
        15348ui64,
        15348ui64
    ) {}
