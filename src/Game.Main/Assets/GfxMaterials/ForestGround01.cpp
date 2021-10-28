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

ForestGround01::ForestGround01() :
        GfxMaterialAsset(
                ForestGround01::auto_guid(),
                image::ForestGround01Albedo::auto_guid(),
                image::ForestGround01AO::auto_guid(),
                image::ForestGround01Cavity::auto_guid(),
                image::ForestGround01Displacement::auto_guid(),
                image::ForestGround01Gloss::auto_guid(),
                image::ForestGround01Normal::auto_guid(),
                image::ForestGround01Roughness::auto_guid(),
                image::ForestGround01Specular::auto_guid()
        ) {}
