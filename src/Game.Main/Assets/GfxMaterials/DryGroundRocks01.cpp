#include "DryGroundRocks01.hpp"

#include "Game.Main/Assets/Textures/DefaultMetalness.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01AO.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Diffuse.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Normal.hpp"
#include "Game.Main/Assets/Textures/DryGroundRocks01Roughness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

DryGroundRocks01::DryGroundRocks01() :
    GfxMaterialAsset(
        DryGroundRocks01::unstable_auto_guid(),
        texture::DryGroundRocks01Diffuse::unstable_auto_guid(),
        texture::DryGroundRocks01AO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::DryGroundRocks01Normal::unstable_auto_guid(),
        texture::DryGroundRocks01Roughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
