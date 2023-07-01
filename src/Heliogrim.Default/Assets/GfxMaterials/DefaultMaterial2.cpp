#include "DefaultMaterial2.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial2::DefaultMaterial2() :
    GfxMaterialAsset(
        DefaultMaterial2::auto_guid<DefaultMaterial2>()
    ) {}
