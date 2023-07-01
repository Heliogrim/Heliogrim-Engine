#include "DefaultSkybox.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DefaultSkybox::DefaultSkybox() :
    GfxMaterialAsset(
        DefaultSkybox::unstable_auto_guid()
    ) {}
