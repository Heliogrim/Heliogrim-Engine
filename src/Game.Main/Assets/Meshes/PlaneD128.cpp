#include "PlaneD128.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::meshes;
using namespace hg;

PlaneD128::PlaneD128() :
    StaticGeometryAsset(
        clone(PlaneD128::unstable_auto_guid()),
        R"(resources\imports\obj\plane_d128.obj)",
        98304uLL,
        98304uLL
    ) {}
