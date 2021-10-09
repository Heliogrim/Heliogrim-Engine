#include "GrassWild01.hpp"

#include "../Images/GrassWild01Albedo.hpp"
#include "../Images/GrassWild01AO.hpp"
#include "../Images/GrassWild01Cavity.hpp"
#include "../Images/GrassWild01Displacement.hpp"
#include "../Images/GrassWild01Gloss.hpp"
#include "../Images/GrassWild01Normal.hpp"
#include "../Images/GrassWild01Roughness.hpp"
#include "../Images/GrassWild01Specular.hpp"

using namespace ember::game::assets::material;
using namespace ember;

const asset_guid GrassWild01::guid = generate_asset_guid();

GrassWild01::GrassWild01() :
    GfxMaterialAsset(
        GrassWild01::guid,
        image::GrassWild01Albedo::guid,
        image::GrassWild01AO::guid,
        image::GrassWild01Cavity::guid,
        image::GrassWild01Displacement::guid,
        image::GrassWild01Gloss::guid,
        image::GrassWild01Normal::guid,
        image::GrassWild01Roughness::guid,
        image::GrassWild01Specular::guid
    ) {}
