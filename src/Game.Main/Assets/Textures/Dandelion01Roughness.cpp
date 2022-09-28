#include "Dandelion01Roughness.hpp"
#include "../Images/Dandelion01Roughness.hpp"

#include <Ember/TextureFormat.hpp>

using namespace ember::game::assets;
using namespace ember;

texture::Dandelion01Roughness::Dandelion01Roughness() :
    TextureAsset(
        texture::Dandelion01Roughness::auto_guid(),
        image::Dandelion01Roughness::auto_guid(),
        {},
        { 8192ui32, 8192ui32, 1ui32 },
        TextureFormat::eR16Sfloat,
        14ui32,
        engine::gfx::TextureType::e2d
    ) { }
