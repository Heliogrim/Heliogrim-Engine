#include "DryGroundRocks01.hpp"

#include "Game.Main/Assets/Textures/DefaultMetalness.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01AO.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Diffuse.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Normal.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Roughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

DryGroundRocks01::DryGroundRocks01() :
    GfxMaterialAsset(
        DryGroundRocks01::auto_guid(),
        texture::DryGroundRocks01Diffuse::auto_guid(),
        texture::DryGroundRocks01AO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::DryGroundRocks01Normal::auto_guid(),
        texture::DryGroundRocks01Roughness::auto_guid(),
        texture::DefaultMetalness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
