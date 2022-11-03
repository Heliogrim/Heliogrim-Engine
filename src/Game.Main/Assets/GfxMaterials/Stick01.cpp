#include "Stick01.hpp"

#include "../Textures/Stick01AO.hpp"
#include "../Textures/Stick01Diffuse.hpp"
#include "../Textures/Stick01Normal.hpp"
#include "../Textures/DefaultRoughness.hpp"
#include "../Textures/DefaultMetalness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

Stick01::Stick01() :
    GfxMaterialAsset(
        Stick01::unstable_auto_guid(),
        texture::Stick01Diffuse::unstable_auto_guid(),
        texture::Stick01AO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Stick01Normal::unstable_auto_guid(),
        texture::DefaultRoughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
