#include "ForestGround01.hpp"

#include "Game.Main/Assets/Textures/ForestGround01AO.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Diffuse.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Normal.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Roughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

ForestGround01::ForestGround01() :
    GfxMaterialAsset(
        ForestGround01::auto_guid(),
        texture::ForestGround01Diffuse::auto_guid(),
        texture::ForestGround01AO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::ForestGround01Normal::auto_guid(),
        texture::ForestGround01Roughness::auto_guid(),
        invalid_asset_guid
    ) {}
