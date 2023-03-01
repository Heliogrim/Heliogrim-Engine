#include "WoodenBucket02.hpp"

#include "../Textures/DefaultAO.hpp"
#include "../Textures/WoodenBucket02Diffuse.hpp"
#include "../Textures/WoodenBucket02Normal.hpp"
#include "../Textures/WoodenBucket02Roughness.hpp"
#include "../Textures/WoodenBucket02Metalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

WoodenBucket02::WoodenBucket02() :
    GfxMaterialAsset(
        WoodenBucket02::unstable_auto_guid(),
        texture::WoodenBucket02Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::WoodenBucket02Normal::unstable_auto_guid(),
        texture::WoodenBucket02Roughness::unstable_auto_guid(),
        texture::WoodenBucket02Metalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
