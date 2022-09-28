#include "Dandelion01.hpp"

#include "../Textures/Dandelion01Alpha.hpp"
#include "../Textures/Dandelion01Diffuse.hpp"
#include "../Textures/Dandelion01Normal.hpp"
#include "../Textures/Dandelion01Roughness.hpp"
#include "../Textures/DefaultAO.hpp"

using namespace ember::game::assets::material;
using namespace ember;

Dandelion01::Dandelion01() :
    GfxMaterialAsset(
        Dandelion01::auto_guid(),
        texture::Dandelion01Diffuse::auto_guid(),
        texture::DefaultAO::auto_guid(),
        invalid_asset_guid,
        invalid_asset_guid,
        invalid_asset_guid,
        texture::Dandelion01Normal::auto_guid(),
        texture::Dandelion01Roughness::auto_guid(),
        invalid_asset_guid,
        texture::Dandelion01Alpha::auto_guid()
    ) {}
