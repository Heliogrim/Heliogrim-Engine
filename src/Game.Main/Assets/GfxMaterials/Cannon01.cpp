#include "Cannon01.hpp"

#include "../Textures/DefaultAO.hpp"
#include "../Textures/Cannon01Diffuse.hpp"
#include "../Textures/Cannon01Normal.hpp"
#include "../Textures/Cannon01Roughness.hpp"
#include "../Textures/Cannon01Metalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

Cannon01::Cannon01() :
    GfxMaterialAsset(
        Cannon01::unstable_auto_guid(),
        texture::Cannon01Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Cannon01Normal::unstable_auto_guid(),
        texture::Cannon01Roughness::unstable_auto_guid(),
        texture::Cannon01Metalness::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid
    ) {}
