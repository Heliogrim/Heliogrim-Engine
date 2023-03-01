#include "ForestGround01.hpp"

#include "Game.Main/Assets/Textures/DefaultMetalness.hpp"
#include "Game.Main/Assets/Textures/ForestGround01AO.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Diffuse.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Normal.hpp"
#include "Game.Main/Assets/Textures/ForestGround01Roughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

ForestGround01::ForestGround01() :
    GfxMaterialAsset(
        ForestGround01::unstable_auto_guid(),
        texture::ForestGround01Diffuse::unstable_auto_guid(),
        texture::ForestGround01AO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::ForestGround01Normal::unstable_auto_guid(),
        texture::ForestGround01Roughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
