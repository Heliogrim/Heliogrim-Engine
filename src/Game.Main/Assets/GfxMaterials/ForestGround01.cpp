#include "ForestGround01.hpp"

#include "Game.Main/Assets/Images/ForestGround01Albedo.hpp"
#include "Game.Main/Assets/Images/ForestGround01AO.hpp"
#include "Game.Main/Assets/Images/ForestGround01Cavity.hpp"
#include "Game.Main/Assets/Images/ForestGround01Displacement.hpp"
#include <Game.Main/Assets/Images/ForestGround01Normal.hpp>
#include <Game.Main/Assets/Images/ForestGround01Gloss.hpp>
#include "Game.Main/Assets/Images/ForestGround01Roughness.hpp"
#include "Game.Main/Assets/Images/ForestGround01Specular.hpp"

using namespace ember::game::assets::material;
using namespace ember;

const asset_guid ForestGround01::guid = generate_asset_guid();

ForestGround01::ForestGround01() :
    GfxMaterialAsset(
        ForestGround01::guid,
        image::ForestGround01Albedo::guid,
        image::ForestGround01AO::guid,
        image::ForestGround01Cavity::guid,
        image::ForestGround01Displacement::guid,
        image::ForestGround01Gloss::guid,
        image::ForestGround01Normal::guid,
        image::ForestGround01Roughness::guid,
        image::ForestGround01Specular::guid
    ) {}
