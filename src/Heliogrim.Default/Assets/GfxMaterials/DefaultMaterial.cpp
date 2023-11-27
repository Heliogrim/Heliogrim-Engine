#include "DefaultMaterial.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial::DefaultMaterial() :
    GfxMaterialAsset(
        DefaultMaterial::auto_guid<DefaultMaterial>(),
        clone(invalid_asset_guid)
    ) {}
