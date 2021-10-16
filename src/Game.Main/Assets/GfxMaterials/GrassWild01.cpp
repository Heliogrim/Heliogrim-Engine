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

GrassWild01::GrassWild01() :
    GfxMaterialAsset(
        GrassWild01::auto_guid(),
        image::GrassWild01Albedo::auto_guid(),
        image::GrassWild01AO::auto_guid(),
        image::GrassWild01Cavity::auto_guid(),
        image::GrassWild01Displacement::auto_guid(),
        image::GrassWild01Gloss::auto_guid(),
        image::GrassWild01Normal::auto_guid(),
        image::GrassWild01Roughness::auto_guid(),
        image::GrassWild01Specular::auto_guid()
    ) {}
