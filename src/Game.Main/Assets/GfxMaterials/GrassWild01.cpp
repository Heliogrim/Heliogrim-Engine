#include "GrassWild01.hpp"

#include "../Images/GrassWild01Albedo.hpp"
#include "../Images/GrassWild01AO.hpp"
#include "../Images/GrassWild01Cavity.hpp"
#include "../Images/GrassWild01Displacement.hpp"
#include "../Images/GrassWild01Gloss.hpp"
#include "../Images/GrassWild01Normal.hpp"
#include "../Images/GrassWild01Roughness.hpp"
#include "../Images/GrassWild01Specular.hpp"
#include "../Textures/DefaultMetalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

GrassWild01::GrassWild01() :
    GfxMaterialAsset(
        GrassWild01::unstable_auto_guid(),
        image::GrassWild01Albedo::unstable_auto_guid(),
        image::GrassWild01AO::unstable_auto_guid(),
        image::GrassWild01Cavity::unstable_auto_guid(),
        image::GrassWild01Displacement::unstable_auto_guid(),
        image::GrassWild01Gloss::unstable_auto_guid(),
        image::GrassWild01Normal::unstable_auto_guid(),
        image::GrassWild01Roughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        image::GrassWild01Specular::unstable_auto_guid(),
        invalid_asset_guid
    ) {}
