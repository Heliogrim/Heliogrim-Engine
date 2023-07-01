#include "DefaultMaterial.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultMaterial::DefaultMaterial() :
    GfxMaterialAsset(
        DefaultMaterial::auto_guid<DefaultMaterial>()
    ) {}
