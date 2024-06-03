#include "DefaultMaterial2.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial2::DefaultMaterial2() :
    GfxMaterialAsset(
        DefaultMaterial2::auto_guid<DefaultMaterial2>(),
        clone(invalid_asset_guid)
    ) {}
