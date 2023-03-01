#include "Rock01.hpp"

#include "../Textures/Rock01Diffuse.hpp"
#include "../Textures/Rock01Normal.hpp"
#include "../Textures/Rock01Roughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

Rock01::Rock01() :
    GfxMaterialAsset(
        Rock01::unstable_auto_guid(),
        texture::Rock01Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Rock01Normal::unstable_auto_guid(),
        texture::Rock01Roughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
