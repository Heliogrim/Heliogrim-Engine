#include "Dandelion01.hpp"

#include "../Textures/Dandelion01Alpha.hpp"
#include "../Textures/Dandelion01Diffuse.hpp"
#include "../Textures/Dandelion01Normal.hpp"
#include "../Textures/Dandelion01Roughness.hpp"
#include "../Textures/DefaultAO.hpp"
#include "../Textures/DefaultMetalness.hpp"

using namespace hg::game::assets::material;
using namespace hg;

Dandelion01::Dandelion01() :
    GfxMaterialAsset(
        Dandelion01::unstable_auto_guid(),
        texture::Dandelion01Diffuse::unstable_auto_guid(),
        texture::DefaultAO::unstable_auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Dandelion01Normal::unstable_auto_guid(),
        texture::Dandelion01Roughness::unstable_auto_guid(),
        texture::DefaultMetalness::unstable_auto_guid(),
        invalid_asset_guid,
        texture::Dandelion01Alpha::unstable_auto_guid()
    ) {}
