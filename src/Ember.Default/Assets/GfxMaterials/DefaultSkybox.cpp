#include "DefaultSkybox.hpp"
#include "../Textures/DefaultSkybox.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"
#include "../Textures/DefaultNormal.hpp"
#include "../Textures/DefaultRoughness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

DefaultSkybox::DefaultSkybox() :
    GfxMaterialAsset(
        DefaultSkybox::unstable_auto_guid(),
        texture::DefaultSkybox::unstable_auto_guid(),
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
