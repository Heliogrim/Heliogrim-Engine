#include "WoodenBucket02.hpp"

using namespace ember::game::assets::meshes;
using namespace ember;

WoodenBucket02::WoodenBucket02() :
    StaticGeometryAsset(
        WoodenBucket02::unstable_auto_guid(),
        R"(resources\imports\obj\wooden_bucket_02.obj)",
        21756ui64,
        21756ui64
    ) {}
