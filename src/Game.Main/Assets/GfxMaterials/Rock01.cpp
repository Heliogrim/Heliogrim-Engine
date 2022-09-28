#include "Rock01.hpp"

#include "../Textures/Rock01Diffuse.hpp"
#include "../Textures/Rock01Normal.hpp"
#include "../Textures/Rock01Roughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"

using namespace ember::game::assets::material;
using namespace ember;

Rock01::Rock01() :
    GfxMaterialAsset(
        Rock01::auto_guid(),
        texture::Rock01Diffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Rock01Normal::auto_guid(),
        texture::Rock01Roughness::auto_guid(),
        texture::DefaultMetalness::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
