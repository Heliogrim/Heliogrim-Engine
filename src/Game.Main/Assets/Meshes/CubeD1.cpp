#include "CubeD1.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::meshes;
using namespace hg;

CubeD1::CubeD1() :
    StaticGeometryAsset(
        clone(CubeD1::unstable_auto_guid())
    ) {}
