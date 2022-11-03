#include "Skybox01.hpp"
#include "Game.Main/Assets/Textures/Skybox01.hpp"

#include "Game.Main/Assets/Textures/DefaultAO.hpp"
#include "Game.Main/Assets/Textures/DefaultMetalness.hpp"
#include "Game.Main/Assets/Textures/DefaultNormal.hpp"
#include "Game.Main/Assets/Textures/DefaultRoughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

Skybox01::Skybox01() :
    GfxMaterialAsset(
        Skybox01::unstable_auto_guid(),
        texture::Skybox01Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::DefaultNormal::unstable_auto_guid(),
        texture::DefaultRoughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
